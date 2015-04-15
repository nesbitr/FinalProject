/* Simple2D Graphics Library for C++
   (c) Katy Coe (sole proprietor) 2012-2013

   No unauthorized copying or distribution. You may re-use Simple2D in your own non-commercial projects.
   For commercial applications, you must obtain a commercial license.

   Support and documentation: www.djkaty.com/simple2d
*/

#include "Simple2DLib.h"

// Intrusive_ptr handling for COM objects
void intrusive_ptr_add_ref(IUnknown *p)
{
	p->AddRef();
}

void intrusive_ptr_release(IUnknown *p)
{
	S2D::SafeRelease(&p);
}

// Hash functions for storing in unordered map
bool operator==(D2D1_COLOR_F const &a, D2D1_COLOR_F const &b)
{
	return a.a == b.a && a.b == b.b && a.g == b.g && a.r == b.r;
}


namespace S2D
{

// Built-in animation algorithms
double Animations::WaitZero(double intervalPc)
{
	return 0.0f;
}

double Animations::WaitOne(double intervalPc)
{
	return 1.0f;
}

double Animations::Linear(double intervalPc)
{
	return intervalPc;
}

double Animations::Sin(double intervalPc)
{
	return std::sin(2 * M_PI * intervalPc);
}

double Animations::Cos(double intervalPc)
{
	return std::cos(2 * M_PI * intervalPc);
}

double Animations::OneMinusSin(double intervalPc)
{
	return 1 - std::sin(2 * M_PI * intervalPc);
}

double Animations::OneMinusCos(double intervalPc)
{
	return 1 - std::cos(2 * M_PI * intervalPc);
}

double Animations::Tan(double intervalPc)
{
	return std::tan(2 * M_PI * intervalPc);
}

double Animations::Log(double intervalPc)
{
	return std::log10(intervalPc);
}

// Animation specializations

TypedAnimation<double>::operator AnimationChain() const
{
	return AnimationChain(*this);
}

// Animation Chain

// Conversion constructor
AnimationChain::AnimationChain(const Animation &a) : onDone(nullptr), paused(true), cycleType(Animation::Clamp), index(0)
{
	Add(a);
}

// Add animation
void AnimationChain::Add(const Animation &a)
{
	animations.push_back(a);
	animations.back().SetCycleType(Animation::Clamp);
	animations.back().Reset(true);

	done = false;
}

// Set callback function for when animation chain completes
void AnimationChain::SetDoneEventHandler(std::function<void (void)> doneFunc)
{
	onDone = doneFunc;
}

// Start or unpause chain
void AnimationChain::Start(bool reset)
{
	paused = false;

	if (index < animations.size())
	{
		if (reset)
			animations[index].Reset();

		done = false;
		animations[index].Pause(false);
	}
	else
	{
		if (cycleType == Animation::Clamp && !done)
		{
			done = true;
			if (onDone)
				onDone();
		}

		if (cycleType == Animation::Repeat)
			Reset();
	}
}

// Reset animation chain (start animation again from the beginning)
void AnimationChain::Reset()
{
	index = 0;
	if (animations.size() > 0)
		Start(true);
}

// Pause animation
void AnimationChain::Pause(bool pause)
{
	if (index < animations.size())
		animations[index].Pause(pause);
		
	if (!pause)
		Start();

	paused = pause;
}

// Return true if the animation is paused
bool AnimationChain::IsPaused() { return paused; }

// Return true if animation chain is finished
bool AnimationChain::Done() { return done; }

// Set cycle type
void AnimationChain::SetCycleType(Animation::CycleType ct) { cycleType = ct; }

// Move to next animation if needed
// Should be called each frame to ensure animations don't stall between transitions
void AnimationChain::Update()
{
	if (index == animations.size())
		return;

	if (animations[index].Done())
	{
		finalValue = animations[index].GetAnimOffset();

		++index;
		Start(true);
	}
}

// Get position in current sub-animation
double AnimationChain::GetAnimOffset(double offset)
{
	Update();

	if (!done)
		return animations[index].GetAnimOffset(offset);

	return finalValue;
}


// Scene management

namespace S2DScene
{

// Scene object base functions

// Bind a variable to an animation, optionally setting it to the master animation
void SceneObject::Bind(SceneObjectBindings bindname, AnimationChain &anim, bool master)
{
	Assert(getBinding(bindname) != 0);

	animations[bindname] = anim;

	if (master)
		masterAnimation = bindname;
}

void SceneObject::Bind(SceneObjectBindings bindname, Animation &anim, bool master)
{
	Bind(bindname, AnimationChain(anim), master);
}

void SceneObject::Unbind(SceneObjectBindings bindname)
{
	animations.erase(bindname);
}

// Get/set the absolute value of a bound variable
double &SceneObject::operator[](SceneObjectBindings bindname)
{
	return *getBinding(bindname);
}

// Set master animation
void SceneObject::SetMasterAnimation(SceneObjectBindings bindname)
{
	masterAnimation = bindname;
}

// Set done behaviour
void SceneObject::SetDeleteBehaviour(SceneObjectDeleteBehaviour sodb)
{
	deleteBehaviour = sodb;
}

// Get done behaviour
SceneObjectDeleteBehaviour SceneObject::GetDeleteBehaviour()
{
	return deleteBehaviour;
}

// Set object owner
void SceneObject::SetOwner(ObjectManager *parent)
{
	owner = parent;
}

void SceneObject::SetOwner(ObjectManager &parent)
{
	owner = &parent;
}

// Get object owner
ObjectManager *SceneObject::GetOwner()
{
	return owner;
}

// Reset all animations to their starting points
void SceneObject::ResetAnimations()
{
	for (auto &kv : animations)
		const_cast<AnimationChain &>(kv.second).Reset();

	done = false;
}

// Drawing function
void SceneObject::Draw()
{
	if (visible)
		doDraw();
}

// Update function
void SceneObject::Update()
{
	for (auto &kv : animations)
	{
		*getBinding(kv.first) = const_cast<AnimationChain &>(kv.second).GetAnimOffset();

		if (masterAnimation == kv.first)
			if (static_cast<AnimationChain>(kv.second).Done())
				done = true;
	}
		
	doUpdate();
}

// Set visible/invisible
void SceneObject::On()
{
	if (visible)
		return;
	
	visible = true;
	doOn();
}

void SceneObject::Off()
{
	if (!visible)
		return;
	
	visible = false;
	doOff();
}

// Return true if the object is currently visible
bool SceneObject::IsOn() { return visible; }

// Return true if object is marked for removal from the scene
bool SceneObject::Done() { return done; }

// Global scene event handling

// Clear the whole scene (but not until the next frame in case we are processing in another thread,
// and don't clear any objects we add to the scene in the same frame after the call to Clear!)
void Scene::Clear()
{
	for (auto &obj : objects)
		markedForRemoval.push_back(&const_cast<SceneObject &>(obj));
}

// Reset the animations for each object
void Scene::ResetAnimations()
{
	for (auto &obj : objects)
		const_cast<SceneObject &>(obj).ResetAnimations();
}

// Update everything in the scene
void Scene::Update()
{
	for (auto &obj : markedForRemoval)
		Remove(*const_cast<SceneObject *>(obj));

	markedForRemoval.clear();

	for (auto &obj : objects)
	{
		SceneObject &objMutable = const_cast<SceneObject &>(obj);

		if (objMutable.IsOn())
		{
			objMutable.Update();
			
			if (objMutable.Done())
				markedForRemoval.push_back(&objMutable);
		}
	}
}

// Draw everything in the scene
void Scene::Draw()
{
	for (auto &obj : objects)
		const_cast<SceneObject &>(obj).Draw();
}

// Called from the Windows message pump when an event occurs
bool Scene::OnKeyCharacter(int key, int rc, bool prev, bool trans)
{
	if (keyCharCallback)
		return keyCharCallback(key, rc, prev, trans);
	
	return false;
}

bool Scene::OnKeyDown(int key, int rc, bool prev)
{
	if (keyDownCallback)
		return keyDownCallback(key, rc, prev);
	
	return false;
}

bool Scene::OnKeyUp(int key, int rc)
{
	if (keyUpCallback)
		return keyUpCallback(key, rc);
	
	return false;
}

bool Scene::OnMouseMove(int x, int y, WPARAM keys)
{
	if (moveCallback)
		if (moveCallback(x, y, keys))
			return true;
	
	return DispatchOnMouseMove(x, y, keys);
}

bool Scene::OnMouseButton(UINT button, int x, int y, WPARAM keys)
{
	if (buttonCallback)
		if (buttonCallback(button, x, y, keys))
			return true;
	
	return DispatchOnMouseButton(button, x, y, keys);
}

// Scene group management
ObjectGroup::ObjectGroup(int x, int y, double a) : SceneObject(), x(0.f), y(0.f), alpha(a)
{
	// Default values
	this->baseX = static_cast<double>(x);
	this->baseY = static_cast<double>(y);
}

double *ObjectGroup::getBinding(SceneObjectBindings b)
{
	switch (b) {
		case BindX: return &this->x;
		case BindY: return &this->y;
		case BindAlpha: return &this->alpha;
		case BindBaseX: return &this->baseX;
		case BindBaseY: return &this->baseY;
	}
	return (double *)0;
}

void ObjectGroup::Clear()
{
	while (objects.size() > 0)
	{
		auto &obj = *objects.begin();
		if (obj.GetDeleteBehaviour() == SODB_Destroy)
			objects.erase(objects.begin());
		else if (obj.GetDeleteBehaviour() == SODB_Release)
			objects.release(objects.begin()).release();
	}
}

void ObjectGroup::ResetAnimations()
{
	SceneObject::ResetAnimations();

	for (auto &obj : objects)
		const_cast<SceneObject &>(obj).ResetAnimations();
}

void ObjectGroup::doUpdate()
{
	for (auto &obj : objects)
	{
		SceneObject &objMutable = const_cast<SceneObject &>(obj);
		objMutable.Update();
	}
}

void ObjectGroup::doDraw()
{
	// Set opacity of all the child objects we render
	renderer->Screen->PushLayer(D2D1::LayerParameters1(
		D2D1::InfiniteRect(), nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::IdentityMatrix(), static_cast<FLOAT>(alpha)), NULL);

	// Set top-left co-ordinate of all the child objects we render
	Matrix wt;
	renderer->Screen->GetTransform(&wt);

	renderer->Screen->SetTransform(wt * D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(baseX + x), static_cast<FLOAT>(baseY + y)));

	// Render objects
	for (auto &obj : objects)
		const_cast<SceneObject &>(obj).Draw();

	// Undo changes to render target
	renderer->Screen->SetTransform(wt);

	renderer->Screen->PopLayer();
}

// Object manager mouse event dispatchers
bool ObjectManager::DispatchOnMouseMove(int x, int y, WPARAM keys)
{
	for (SceneObject &o : objects)
	{
		auto i = dynamic_cast<InterfaceObject *>(&o);
		if (i)
			if (i->OnMouseMove(x, y, keys))
				return true;

		auto i2 = dynamic_cast<ObjectGroup *>(&o);
		if (i2)
		{
			if (i2->IsOn())
				if (i2->DispatchOnMouseMove(x - static_cast<int>((*i2)[BindBaseX] - (*i2)[BindX]), y - static_cast<int>((*i2)[BindBaseY] - (*i2)[BindY]), keys))
					return true;
		}
		else
		{
			auto i3 = dynamic_cast<ObjectManager *>(&o);
			if (i3)
				if (i3->DispatchOnMouseMove(x, y, keys))
					return true;
		}
	}
	return false;
}

bool ObjectManager::DispatchOnMouseButton(UINT button, int x, int y, WPARAM keys)
{
	for (SceneObject &o : objects)
	{
		auto i = dynamic_cast<InterfaceObject *>(&o);
		if (i)
			if (i->OnMouseButton(button, x, y, keys))
				return true;

		auto i2 = dynamic_cast<ObjectGroup *>(&o);
		if (i2)
		{
			if (i2->IsOn())
				if (i2->DispatchOnMouseButton(button, x - static_cast<int>((*i2)[BindBaseX] - (*i2)[BindX]), y - static_cast<int>((*i2)[BindBaseY] - (*i2)[BindY]), keys))
					return true;
		}
		else
		{
			auto i3 = dynamic_cast<ObjectManager *>(&o);
			if (i3)
				if (i3->DispatchOnMouseButton(button, x, y, keys))
					return true;
		}
	}
	return false;
}

// Concrete scene objects

// Rectangle
#ifdef SIMPLE2D_COMPAT_107
Rectangle::Rectangle(Simple2D *r, int x, int y, int w, int h, GenericBrush *b)
	: Rectangle(x, y, w, h, b) {}
#endif

Rectangle::Rectangle(int x, int y, int w, int h, GenericBrush *b) : SceneObject(), brush(b), alpha(-1)
{
	// Default values
	this->x = static_cast<double>(x);
	this->y = static_cast<double>(y);
	this->w = static_cast<double>(w);
	this->h = static_cast<double>(h);
}

double *Rectangle::getBinding(SceneObjectBindings b)
{
	switch (b) {
		case BindX: return &this->x;
		case BindY: return &this->y;
		case BindWidth: return &this->w;
		case BindHeight: return &this->h;
		case BindAlpha: return &this->alpha;
	}
	return (double *)0;
}

void Rectangle::doDraw()
{
	if (alpha == -1)
		alpha = brush->GetBrush()->GetOpacity();

	brush->SetOpacity(static_cast<FLOAT>(alpha));
	renderer->FillRectangleWH(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h), brush);
}

// Text label
#ifdef SIMPLE2D_COMPAT_107
Label::Label(Simple2D *r, int x, int y, string text, GenericBrush *b, TextFormat fmt, int w, int h)
	: Label(x, y, text, b, fmt, w, h) {}
#endif

Label::Label(int x, int y, string text, GenericBrush *b, TextFormat fmt, int w, int h)
	: SceneObject(), text(text), brush(b), format(fmt), w(w), h(h), alpha(-1)
{
	// Default values
	this->x = static_cast<double>(x);
	this->y = static_cast<double>(y);
}

double *Label::getBinding(SceneObjectBindings b)
{
	switch (b) {
		case BindX: return &this->x;
		case BindY: return &this->y;
		case BindAlpha: return &this->alpha;
	}
	return (double *)0;
}

void Label::doDraw()
{
	if (alpha == -1)
		alpha = brush->GetBrush()->GetOpacity();

	brush->SetOpacity(static_cast<FLOAT>(alpha));
	renderer->Text(static_cast<int>(x), static_cast<int>(y), text, brush, format, w, h);
}

// Sprite
#ifdef SIMPLE2D_COMPAT_107
Sprite::Sprite(Simple2D *r, ImageObject *img, int x, int y, int w, int h)
	: Sprite(img, x, y, w, h) {}
#endif

Sprite::Sprite(ImageObject *img, int x, int y, int w, int h)
	: SceneObject(), image(img), x(x), y(y), w(w), h(h), sx(0), sy(0), sw(-1), sh(-1), alpha(1.0f), rotation(0.0f) {}

double *Sprite::getBinding(SceneObjectBindings b)
{
	switch (b) {
		case BindX: return &this->x;
		case BindY: return &this->y;
		case BindWidth: return &this->w;
		case BindHeight: return &this->h;
		case BindX2: return &this->sx;
		case BindY2: return &this->sy;
		case BindWidth2: return &this->sw;
		case BindHeight2: return &this->sh;
		case BindAlpha: return &this->alpha;
		case BindRotation: return &this->rotation;
	}
	return (double *)0;
}

void Sprite::doDraw()
{
	D2D1_SIZE_F size;
	size = image->GetImage()->GetSize();

	if (w == -1)
		w = size.width;

	if (h == -1)
		h = size.height;

	if (sw == -1)
		sw = size.width;

	if (sh == -1)
		sh = size.height;

	image->DrawPartWH(	static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h),
						static_cast<int>(sx), static_cast<int>(sy), static_cast<int>(sw), static_cast<int>(sh),
						static_cast<float>(alpha), static_cast<float>(rotation));
}

// Set render rectangle
void Sprite::SetRectWH(int x, int y, int w, int h)
{
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
}

// Set source rectangle
void Sprite::SetSourceRectWH(int x, int y, int w, int h)
{
	sx = x;
	sy = y;
	sw = w;
	sh = h;
}

// Sprite sheet
#ifdef SIMPLE2D_COMPAT_107
SpriteSheet::SpriteSheet(Simple2D *r, ImageObject *img, int col, int row, int item)
	: SpriteSheet(img, col, row, item) {}
#endif

SpriteSheet::SpriteSheet(ImageObject *img, int col, int row, int item, int frames)
	: Sprite(img), cols(col), rows(row), selectedImage(item), useAnimation(false), animationChanged(false)
{
	this->frames = (frames == -1? col * row : frames);
}

// Set sprite to plot
void SpriteSheet::SetSprite(int s)
{
	useAnimation = false;
	selectedImage = s;
}

// Select to plot sprite based on animation result
void SpriteSheet::SetSpriteFromAnimation(bool use)
{
	useAnimation = use;
	animationChanged = use;
}

// Set animation function to control which sprite is used from the sheet
// Returns values from 0.0 to 1.0 which are scaled to the available number of sprites
void SpriteSheet::SetAnimation(AnimationChain &a)
{
	spriteSelector = a;
	SetSpriteFromAnimation();
}

void SpriteSheet::SetAnimation(Animation &a, Animation::CycleType ct)
{
	spriteSelector = AnimationChain(a);
	spriteSelector.SetCycleType(ct);
	SetSpriteFromAnimation();
}

// Drawing function
void SpriteSheet::doDraw()
{
	if (animationChanged)
	{
		spriteSelector.Start();
		animationChanged = false;
	}

	if (useAnimation)
		selectedImage = max(min(static_cast<int>(spriteSelector.GetAnimOffset() * frames), frames - 1), 0);

	D2D1_SIZE_F size;
	size = image->GetImage()->GetSize();

	int col = selectedImage % cols;
	int row = selectedImage / cols;

	int tileX = col * static_cast<int>(size.width / static_cast<float>(cols));
	int tileY = row * static_cast<int>(size.height / static_cast<float>(rows));

	// Use the selected sprite unless overridden by the user
	if (sx == -1 || sy == -1 || sw == -1 || sh == -1)
	{
		SetSourceRectWH(tileX, tileY, static_cast<int>(size.width / static_cast<float>(cols)), static_cast<int>(size.height / static_cast<float>(rows)));
		Sprite::doDraw();
		sx = sy = sw = sh = -1;
	}
	else
		Sprite::doDraw();
}

// Interface object helpers

// Legacy constructors
#ifdef SIMPLE2D_COMPAT_107
InterfaceObject::InterfaceObject(Simple2D *r, int x, int y, int w, int h, Simple2DMouseButtonFunc bf)
	: InterfaceObject(x, y, w, h, nullptr, nullptr, bf, nullptr, nullptr, nullptr) {}
InterfaceObject::InterfaceObject(Simple2D *r, int x, int y, int w, int h, Simple2DMouseHoverFunc hf, Simple2DMouseHoverFunc uhf, Simple2DMouseButtonFunc bf)
	: InterfaceObject(x, y, w, h, hf, uhf, bf, nullptr, nullptr, nullptr) {}
InterfaceObject::InterfaceObject(Simple2D *r, Simple2DKeyCharFunc cf, Simple2DKeyDownFunc df, Simple2DKeyUpFunc uf)
	: InterfaceObject(0, 0, 100000, 100000, nullptr, nullptr, nullptr, cf, df, uf) {}
#endif

// New constructors (>= 1.08)
InterfaceObject::InterfaceObject(int x, int y, int w, int h)
	: InterfaceObject(x, y, w, h, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) {}
InterfaceObject::InterfaceObject(int x, int y, int w, int h, Simple2DMouseButtonFunc bf)
	: InterfaceObject(x, y, w, h, nullptr, nullptr, bf, nullptr, nullptr, nullptr) {}
InterfaceObject::InterfaceObject(int x, int y, int w, int h, Simple2DMouseHoverFunc hf, Simple2DMouseHoverFunc uhf, Simple2DMouseButtonFunc bf)
	: InterfaceObject(x, y, w, h, hf, uhf, bf, nullptr, nullptr, nullptr) {}
InterfaceObject::InterfaceObject(Simple2DKeyCharFunc cf, Simple2DKeyDownFunc df, Simple2DKeyUpFunc uf)
	: InterfaceObject(0, 0, 100000, 100000, nullptr, nullptr, nullptr, cf, df, uf) {}

InterfaceObject::InterfaceObject(int x, int y, int w, int h,
							 Simple2DMouseHoverFunc hf, Simple2DMouseHoverFunc uhf, Simple2DMouseButtonFunc bf,
							 Simple2DKeyCharFunc cf, Simple2DKeyDownFunc df, Simple2DKeyUpFunc uf)
							 : SceneObject(), x(x), y(y), w(w), h(h),
							   hoverCallback(hf), unhoverCallback(uhf), buttonCallback(bf),
							   keyCharCallback(cf), keyDownCallback(df), keyUpCallback(uf),
							   mouseOver(false), justGotFocus(false)
{
	if (uhf)
		uhf();
}

bool InterfaceObject::OnWindowsMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!visible)
		return false;

	return doOnWindowsMessage(hwnd, message, wParam, lParam);
}

bool InterfaceObject::OnMouseMove(int x, int y, WPARAM wParam)
{
	if (!visible)
		return false;

	if (moveCallback)
		if (moveCallback(x, y, wParam))
			return true;

	bool wasMouseOver = mouseOver;

	// Figure out if the mouse is in the bounding box
	mouseOver = (x >= this->x && x < this->x + w && y >= this->y && y < this->y + h);

	// If it is but wasn't before, run the callback
	if (mouseOver && !wasMouseOver)
	{
		doOnMouseHover(x, y, wParam);
		if (hoverCallback)
			hoverCallback();
	}

	// If it isn't but was before, run the callback
	if (!mouseOver && wasMouseOver)
	{
		doOnMouseUnhover(x, y, wParam);
		if (unhoverCallback)
			unhoverCallback();
	}

	if (!mouseOver)
		return false;

	return doOnMouseMove(x, y, wParam);
}

bool InterfaceObject::OnMouseButton(UINT button, int x, int y, WPARAM wParam)
{
	if (!visible)
		return false;

	// Re-calculate mouseOver in case we are not tracking hovers
	OnMouseMove(x, y, wParam);

	if (mouseOver)
	{
		// Give control focus if left-clicked
		if (button == WM_LBUTTONDOWN)
		{
			renderer->SetFocusObject(this);
			justGotFocus = true;
		}

		if (buttonCallback)
			if (buttonCallback(button, x, y, wParam))
				return true;

		return (doOnMouseButton(button, x, y, wParam) || justGotFocus);
	}
	return false;
}

bool InterfaceObject::OnKeyCharacter(int key, int rc, bool prev, bool trans)
{
	if (!visible)
		return false;

	if (keyCharCallback)
		if (keyCharCallback(key, rc, prev, trans))
			return true;

	return doOnKeyCharacter(key, rc, prev, trans);
}

bool InterfaceObject::OnKeyDown(int key, int rc, bool prev)
{
	if (!visible)
		return false;

	// This piece of fudge prevents the first keypress being processed as a character
	// if a key is already being held down when a text control receives focus
	if (justGotFocus && prev)
		return true;

	if (justGotFocus && !prev)
		justGotFocus = false;

	// Tabbed item handling - note, we must be the focus object to get here in the first place
	if (key == VK_TAB)
	{
		// Find out if the owner is a tabbed item group
		auto *owner = dynamic_cast<ITabbedObjectGroup *>(GetOwner());
		if (owner)
		{
			// Forwards or backwards in tab order
			bool previous = (GetAsyncKeyState(VK_SHIFT) != 0);

			// Update focus object
			Simple2D::App->SetFocusObject(dynamic_cast<InterfaceObject *>(owner->FindTabbedItem(!previous)));

			return true;
		}
	}

	if (keyDownCallback)
		if (keyDownCallback(key, rc, prev))
			return true;

	return doOnKeyDown(key, rc, prev);
}

bool InterfaceObject::OnKeyUp(int key, int rc)
{
	if (!visible)
		return false;

	if (keyUpCallback)
		if (keyUpCallback(key, rc))
			return true;

	return doOnKeyUp(key, rc);
}

void InterfaceObject::doOn()
{
	doOnMouseUnhover(-1, -1, -1);
	
	if (unhoverCallback)
		unhoverCallback();
	
	mouseOver = false;
}

void InterfaceObject::doOff()
{
	doOnMouseUnhover(-1, -1, -1);
	
	if (unhoverCallback)
		unhoverCallback();
	
	mouseOver = false;
	
	renderer->SetFocusObject(this, false);
	justGotFocus = false;
}

void InterfaceObject::SetFocus(bool focus)
{
	justGotFocus = focus;
	renderer->SetFocusObject(this, focus);
}

bool InterfaceObject::HasFocus()
{
	return renderer->GetFocusObject() == this;
}

// Tabbed groups
ITabbedItem *ObjectGroup::FindTabbedItem(bool forwards)
{
	// Do nothing if group is not visible
	if (!visible)
		return nullptr;

	// Do nothing if group is empty
	if (objects.size() == 0)
		return nullptr;

	// Set start and end search ranges
	auto begin = objects.begin();
	auto end   = objects.end();

	// Get global focus object
	auto *focus = dynamic_cast<ITabbedItem *>(Simple2D::App->GetFocusObject());

	boost::ptr_list<SceneObject>::iterator it;

	// Was a tabbed item, find it in the list
	if (focus)
	{
		it = find_ptr(objects.begin(), objects.end(), Simple2D::App->GetFocusObject());

		// Object is no longer in (or never was in) this object group, so select first object
		if (it == objects.end())
		{
			if (forwards)
				it = begin;
			else
			{
				it = end;
				--it;
			}
			focus = nullptr;
		}

		// Otherwise advanced by one
		else
		{
			if (forwards)
				++it;
			else
				--it;
		}
	}

	// Wasn't a tabbed item, select first item in list
	else
	{
		if (forwards)
			it = begin;
		else
		{
			it = end;
			--it;
		}
		focus = nullptr;
	}

	auto found = end;
	ITabbedItem *obj = nullptr;

	do
	{
		// Wrap around if we reached the end of the list
		if (it == end)
		{
			if (forwards)
				it = begin;
			else
				--it;
		}

		// Check if the item being examined is a tabbed item
		obj = dynamic_cast<ITabbedItem *>(&(*it));
		if (obj)
		{
			// Check if object is visible
			if (it->IsOn())
				found = it;
		}

		// Increment list position
		if (forwards)
			++it;
		else
		{
			if (it != begin)
				--it;
			else
				it = end;
		}

		// Continue while...
	} while (found == end &&						// We didn't find a tabbable object yet, AND:
			(focus != nullptr && obj != focus)				// EITHER: we haven't wrapped around to found the original object
		||  (focus == nullptr && it != end));     // OR: the original object wasn't in the list and we haven't scanned the whole list

	// New object found
	if (found != end)
		return dynamic_cast<ITabbedItem *>(&(*found));

	// Only tabbable item is existing item
	if (focus != nullptr && obj == focus)
		return focus;

	return nullptr;
}

// Text control helper class
void TextBox::SetCanvas(int xo, int yo, int xr, int yr, GenericBrush *ob, GenericBrush *fb)
{
	canvasXoffset = xo;
	canvasYoffset = yo;
	canvasXradius = xr;
	canvasYradius = yr;
	canvasOutlineBrush = ob;
	canvasFillBrush = fb;
}

void TextBox::SetCaption(string text, TextFormat format, GenericBrush *brush, TextBoxCaptionPosition position, int gap)
{
	Caption = text;
	captionFormat = format;
	captionBrush = brush;
	captionPosition = position;
	captionGap = gap;
}

void TextBox::SetSelectionBrushes(GenericBrush *stb, GenericBrush *sbb)
{
	selTextBrush = stb;
	selBoxBrush = sbb;
}

void TextBox::doOn()
{
	InterfaceObject::doOn();

	ReturnPressed = false;
}

void TextBox::doOff()
{
	InterfaceObject::doOff();

	ReturnPressed = false;
}

bool TextBox::doOnMouseButton(UINT, int, int, WPARAM)
{
	ReturnPressed = false;

	return false;
}

bool TextBox::doOnKeyUp(int key, int rc)
{
	// SHIFT
	if (key == VK_SHIFT)
	{
		shiftPressed = false;
		return true;
	}

	// CTRL
	if (key == VK_CONTROL)
	{
		ctrlPressed = false;
		return true;
	}

	return false;
}

bool TextBox::doOnKeyDown(int key, int rc, bool prev)
{
	// SHIFT
	if (key == VK_SHIFT)
	{
		shiftPressed = true;
		return true;
	}

	// CTRL
	if (key == VK_CONTROL)
	{
		ctrlPressed = true;
		return true;
	}

	// Navigation
	if (key == VK_LEFT)
	{
		// Selection cancelled; move caret to left of selection
		if (!shiftPressed && selectionPoint != string::npos)
		{
			Caret = min(Caret, selectionPoint);
			selectionPoint = string::npos;
			return true;
		}

		// Do nothing else if caret on left-hand side
		if (Caret == 0)
			return false;

		// New selection starting
		if (shiftPressed && selectionPoint == string::npos)
			selectionPoint = Caret;

		// Move left by one character
		if (!ctrlPressed)
			Caret--;

		// Move left by one word
		else
		{
			unsigned int p = Text.substr(0, Caret - 1).find_last_of(" ");
			Caret = (p == string::npos? 0 : p + 1);
		}

		// Selection reversed?
		if (Caret == selectionPoint)
			selectionPoint = string::npos;

		return true;
	}

	if (key == VK_RIGHT)
	{
		// Selection cancelled; move caret to right of selection
		if (!shiftPressed && selectionPoint != string::npos)
		{
			Caret = max(Caret, selectionPoint);
			selectionPoint = string::npos;
			return true;
		}

		// Do nothing else if caret on right-hand side
		if (Caret == Text.length())
			return false;

		// New selection starting
		if (shiftPressed && selectionPoint == string::npos)
			selectionPoint = Caret;

		// Move right by one character
		if (!ctrlPressed)
			Caret++;

		// Move right by one word
		else
		{
			unsigned int p = Text.find_first_of(" ", Caret);
			Caret = (p == string::npos? Text.length() : p + 1);
		}

		// Selection reversed?
		if (Caret == selectionPoint)
			selectionPoint = string::npos;

		return true;
	}

	if (key == VK_HOME)
	{
		if (shiftPressed)
		{
			if (selectionPoint == string::npos && Caret > 0)
				selectionPoint = Caret;

			else if (selectionPoint == 0)
				selectionPoint = string::npos;
		}
		else
			selectionPoint = string::npos;

		Caret = 0;
		return true;
	}

	if (key == VK_END)
	{
		if (shiftPressed)
		{
			if (selectionPoint == string::npos && Caret < Text.length())
				selectionPoint = Caret;

			else if (selectionPoint == Text.length())
				selectionPoint = string::npos;
		}
		else
			selectionPoint = string::npos;

		Caret = Text.length();
		return true;
	}

	// Delete character after the caret
	if (key == VK_DELETE)
	{
		if (selectionPoint == string::npos)
		{
			if (Caret < Text.length())
			{
				push(TBSA_DeleteForward);

				Text.erase(Caret, 1);
			}
		}

		// Delete selection
		else
			deleteSelection();

		return true;
	}

	// Select all
	if (key == 'A' && ctrlPressed)
	{
		selectionPoint = 0;
		Caret = Text.length();
	}

	// Clipboard support
	if (key == 'X' && ctrlPressed)
	{
		SendMessage(Simple2D::App->GetWindow(), WM_COMMAND, IOWC_CUT, 1);
		return true;
	}

	if (key == 'C' && ctrlPressed)
	{
		SendMessage(Simple2D::App->GetWindow(), WM_COMMAND, IOWC_COPY, 1);
		return true;
	}

	if (key == 'V' && ctrlPressed)
	{
		SendMessage(Simple2D::App->GetWindow(), WM_COMMAND, IOWC_PASTE, 1);
		return true;
	}

	// Undo/redo
	if (key == 'Z' && ctrlPressed && !shiftPressed)
	{
		undo();
		return true;
	}

	if ((key == 'Y' && ctrlPressed) || (key == 'Z' && shiftPressed && ctrlPressed))
	{
		redo();
		return true;
	}

	return false;
}

bool TextBox::doOnKeyCharacter(int key, int rc, bool prev, bool trans)
{
	if (key >= 32 && !justGotFocus)
	{
		// Delete the selection first if there is one so we can replace it
		deleteSelection();

		if (Text.length() < maxlen)
		{
			push(TBSA_Add);

			Text = Text.substr(0, Caret) + static_cast<char>(key) + Text.substr(Caret);
			Caret++;
		}

		justGotFocus = false;
		return true;
	}

	// Delete character before the caret
	if (key == VK_BACK)
	{
		if (selectionPoint == string::npos)
		{
			if (Caret > 0)
			{
				push(TBSA_DeleteBack);

				// If we delete from the middle of the text with both edges outside the box, we want to pin
				// the caret to roughly where it is now
				if (Caret < Text.length() && charStart > 0)
					charStart--;

				Text.erase(Caret - 1, 1);
				Caret--;
			}
		}
		
		// Delete selection
		else
			deleteSelection();

		return true;
	}

	if (key == VK_RETURN)
	{
		if (Text.length() > 0)
		{
			renderer->SetFocusObject(this, false);
			ReturnPressed = true;
		}
		return true;
	}
	return false;
}

bool TextBox::doOnWindowsMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message != WM_COMMAND)
		return false;

	bool done = false;

	int selStart = min(selectionPoint, Caret);
	int selEnd = max(selectionPoint, Caret);

	size_t size = (selEnd - selStart + 1) * sizeof(WCHAR);

	HGLOBAL hglb;
	LPWSTR lpwstrCopy;
	LPSTR lpstrCopy;
	WCHAR *conv;

	switch (LOWORD(wParam))
	{
	case IOWC_CUT:
	case IOWC_COPY:
		// Is there anything to cut/copy?
		if (selectionPoint == string::npos)
			break;

		// Don't allow cutting or copying of passwords
		if (IsPassword)
			break;

		// Open the clipboard
		if (!OpenClipboard(hwnd))
			return false;

		// Empty the clipboard
		EmptyClipboard();

		// Allocate shared memory for the text
		if ((hglb = GlobalAlloc(GMEM_MOVEABLE, size)) == NULL)
		{
			CloseClipboard();
			return false;
		}

		// Convert the text to Unicode WCHAR * and copy it to the shared memory
		lpwstrCopy = (LPWSTR) GlobalLock(hglb);
		conv = StringToWCHAR(Text.substr(selStart, selEnd - selStart));
		memcpy(lpwstrCopy, conv, (selEnd - selStart) * sizeof(WCHAR));
		lpwstrCopy[selEnd - selStart] = (WCHAR) 0;
		delete [] conv;
		GlobalUnlock(hglb);

		// Save it to the clipboard, freeing up the memory if the save fails
		if (SetClipboardData(CF_UNICODETEXT, hglb) == NULL)
			GlobalFree(hglb);

		// Close the clipboard
		CloseClipboard();

		// If we did Cut, delete the selection
		if (LOWORD(wParam) == IOWC_CUT)
		{
			push(TBSA_DeleteBack, true);

			Text.erase(selStart, selEnd - selStart);
			Caret = selStart;
			selectionPoint = string::npos;
		}

		done = true;
		break;

	case IOWC_PASTE:
		// See if there is any text on the clipboard
		if (!IsClipboardFormatAvailable(CF_UNICODETEXT) && !IsClipboardFormatAvailable(CF_TEXT))
			return false;

		// Open the clipboard
		if (!OpenClipboard(hwnd))
			return false;
		
		// Prefer Unicode version
		if (IsClipboardFormatAvailable(CF_UNICODETEXT))
		{
			hglb = GetClipboardData(CF_UNICODETEXT);
			if (hglb != NULL)
			{
				lpwstrCopy = (LPWSTR) GlobalLock(hglb);
				if (lpwstrCopy != NULL)
				{
					// Get the text on the clipboard
					string textToPaste = StringFactory(lpwstrCopy);

					// Delete selection if there is one
					deleteSelection();

					// Insert into text
					push(TBSA_Add, true);

					Text = Text.substr(0, Caret) + textToPaste + Text.substr(Caret);
					Caret += textToPaste.length();

					// Crop the end of the text if necessary
					if (Text.length() > maxlen)
						Text = Text.substr(0, maxlen);

					if (Caret > Text.length())
						Caret = Text.length();

					GlobalUnlock(hglb);
				}
			}
		}
		
		else if (IsClipboardFormatAvailable(CF_TEXT))
		{
			hglb = GetClipboardData(CF_TEXT);
			if (hglb != NULL)
			{
				lpstrCopy = (LPSTR) GlobalLock(hglb);
				if (lpstrCopy != NULL)
				{
					// Get the text on the clipboard
					string textToPaste = StringFactory(lpstrCopy);

					// Delete selection if there is one
					deleteSelection();

					// Insert into text
					push(TBSA_Add, true);

					Text = Text.substr(0, Caret) + textToPaste + Text.substr(Caret);
					Caret += textToPaste.length();

					// Crop the end of the text if necessary
					if (Text.length() > maxlen)
						Text = Text.substr(0, maxlen);

					if (Caret > Text.length())
						Caret = Text.length();

					GlobalUnlock(hglb);
				}
			}
		}
		
		CloseClipboard();

		done = true;
		break;
	}

	return done;
}

void TextBox::doDraw()
{
	// Put caret at end of text if we just got focus
	if (justGotFocus)
	{
		shiftPressed = false;
		ctrlPressed = false;
		selectionPoint = string::npos;
		Caret = Text.length();
	}

	// Initialize selection brushes if none specified
	if (!selTextBrush)
		selTextBrush = renderer->MakeBrush(Colour::Black);

	if (!selBoxBrush)
		selBoxBrush = renderer->MakeBrush(Colour::White);

	// Remove selection if focus lost
	if (!HasFocus())
		selectionPoint = string::npos;

	// Draw canvas
	if (canvasFillBrush)
		renderer->FillRoundedRectangleWH(x - canvasXoffset, y - canvasYoffset, w + canvasXoffset * 2, h + canvasYoffset * 2,
			canvasXradius, canvasYradius, canvasFillBrush);

	if (canvasOutlineBrush)
		renderer->DrawRoundedRectangleWH(x - canvasXoffset, y - canvasYoffset, w + canvasXoffset * 2, h + canvasYoffset * 2,
			canvasXradius, canvasYradius, canvasOutlineBrush);

	// Draw caption
	if (Caption.length() > 0)
	{
		layout = renderer->MakeTextLayout(Caption, captionFormat);
		int width = renderer->TextWidth(layout);
		int height = renderer->TextHeight(layout);

		if (captionPosition == TBCP_Left)
			renderer->Text(x - width - captionGap, y + (h - height) / 2, Caption, captionBrush, captionFormat);

		else if (captionPosition == TBCP_Above)
		{
			renderer->Text(x, y - captionGap - height, Caption, captionBrush, captionFormat);
		}
	}

	// Substitutions before rendering text
	string textToRender = (!IsPassword? Text : string(Text.length(), '*'));

	// If the caret is off the left of the text box (due to left navigation),
	// update the first character to render accordingly to bring the caret back into the box
	if (Caret < charStart)
		charStart = Caret;

	// Go through all the characters until the box is full, preferring right-justification
	charStart++;
	do
	{
		charStart--;
		layout = renderer->MakeTextLayout(textToRender.substr(charStart), format);
	} while (renderer->TextWidth(layout) < w && charStart > 0);

	// Go through all the characters until the caret is inside the right-hand edge of the box
	do
	{
		layout = renderer->MakeTextLayout(textToRender.substr(charStart, Caret - charStart), format);
		charStart++;
	} while (renderer->TextWidth(layout) >= w);
	charStart--;

	// Find out how many characters to render to fit the box perfectly
	int tw = 0, numToRender = 0;

	while (tw < w && charStart + numToRender < Text.length())
	{
		layout = renderer->MakeTextLayout(textToRender.substr(charStart, ++numToRender), format);
		tw = renderer->TextWidth(layout);
	}
	if (tw >= w)
		numToRender--;

	// Debugging: show undo buffer
	/*
	if (undoHistory.empty())
		undoPos = undoHistory.end();

	int uy = 300;
	for (TextBoxState &ts : undoHistory)
	{
		renderer->Text(20, uy, StringFactory(ts.caret) + " " + StringFactory(ts.charStart) + " " + StringFactory(ts.seal) + " " + ts.text, textBrush, renderer->MakeTextFormat(L"Verdana", 14.0f));

		if (undoPos != undoHistory.end())
			if (&(*undoPos) == &ts)
				renderer->Text(5, uy, "*", textBrush, renderer->MakeTextFormat(L"Verdana", 14.0f));

		uy += 20;
	}

	if (undoPos == undoHistory.end())
		renderer->Text(5, uy, "*", textBrush, renderer->MakeTextFormat(L"Verdana", 14.0f));
	*/

	// Draw text when there is no selection
	if (selectionPoint == string::npos)
		renderer->Text(x, y, textToRender.substr(charStart, numToRender), textBrush, format, w, h);

	// Draw text when there is a selection
	if (selectionPoint != string::npos)
	{
		// Get start and end+1 characters of selection
		int selStart = min(selectionPoint, Caret);
		int selEnd   = max(selectionPoint, Caret);

		// Calculate number of unselected characters to display before and after the selection
		unsigned int nBeforeSel = max(min(selStart - (int) charStart, numToRender), 0);
		unsigned int nAfterSel = max(min(numToRender - (selEnd - (int) charStart), numToRender), 0);

		// Calculate number of selected characters to display
		unsigned int nDuringSel = numToRender - (nBeforeSel + nAfterSel);

		// Starting x position
		int curX = x;

		// Draw
		if (nBeforeSel > 0)
		{
			renderer->Text(curX, y, textToRender.substr(charStart, nBeforeSel), textBrush, format, w, h);
			layout = renderer->MakeTextLayout(textToRender.substr(charStart, nBeforeSel), format);
			curX += renderer->TextWidth(layout);
		}

		if (nDuringSel > 0)
		{
			layout = renderer->MakeTextLayout(textToRender.substr(charStart + nBeforeSel, nDuringSel), format);

			// Draw background
			renderer->FillRectangleWH(curX, y, renderer->TextWidth(layout), h, selBoxBrush);

			// Draw text
			renderer->Text(curX, y, textToRender.substr(charStart + nBeforeSel, nDuringSel), selTextBrush, format, w, h);
			curX += renderer->TextWidth(layout);
		}

		if (nAfterSel > 0)
		{
			renderer->Text(curX, y, textToRender.substr(charStart + nBeforeSel + nDuringSel, nAfterSel), textBrush, format, w, h);
		}
	}

	// Draw caret if control has focus and there is no selection
	if (HasFocus() && selectionPoint == string::npos)
	{
		// Get caret position
		layout = renderer->MakeTextLayout(textToRender.substr(charStart, Caret - charStart), format, w, h);
		int caretPos = renderer->TextWidth(layout);

		// Draw caret
		renderer->FillRectangleWH(x + caretPos, y + caretYoffset, 2, h - caretYheightOffset, caretBrush);
	}
}

// NOTE: The undo code is a bit buggy but more or less works
// Uncomment the debugging section in TextBox::doDraw() to monitor the undo buffer
void TextBox::push(TextBoxStateAction action, bool forceSeal)
{
	static int caretPrevious = -100;

	bool create = false;
	bool sealLast = false;

	// Validate undoPos
	// Rationale: TextBoxes are shallow copy constructed on creation sometimes.
	// The list copies fine, the iterator ends up pointing to the source's list;
	// but on creation, the list will be empty, so we can check for this
	// situation here.
	if (undoHistory.empty())
		undoPos = undoHistory.end();

	// Remove everything from current history position + 1 to the end
	if (undoPos != undoHistory.end())
	{
		//create = true;
		undoHistory.erase(undoPos, undoHistory.end());
		undoPos = undoHistory.end();
		//sealLast = true;
	}

	// Create a new history item if:
	// 1. the history is empty
	// 2. the previous item is sealed
	// 3. this item must be sealed (and separate)
	// 4. the action type of this event is different to the last one in the history
	// 5. TBSA_Add but caret is not 1 position right of the last event
	// 6. TBSA_DeleteBack but caret is not 1 position left of the last event
	// 7. TBSA_DeleteForward but caret is not at same position of the last event
	// 8. If the current history pointer is not the end of the history
	// (meaning some things have been undone but not redone)

	/*else*/ if (undoHistory.empty())
		create = true;

	else
	{
		UndoIterator last = undoPos;
		last--;

		if (forceSeal)
			create = sealLast = true;

		else if (last->seal)
			create = true;

		else if (last->thisAction != action)
			create = sealLast = true;

		else
		{
			switch (action) {
			case TBSA_Add:
				if (Caret != caretPrevious + 1 && last->text != Text)
					create = sealLast = true;
				break;

			case TBSA_DeleteBack:
				if (Caret != caretPrevious - 1 && last->text != Text)
					create = sealLast = true;
				break;

			case TBSA_DeleteForward:
				if (Caret != caretPrevious && last->text != Text)
					create = sealLast = true;
				break;
			}
		}
	}

	// In cases 3-8, seal the last history item
	if (sealLast && !undoHistory.empty())
	{
		UndoIterator toSeal = undoPos;
		toSeal--;
		toSeal->seal = true;
	}

	// Store caret position
	caretPrevious = Caret;

	// Do creation of new item
	if (create)
	{
		TextBoxState s;
		s.caret = Caret;
		s.charStart = charStart;
		s.seal = forceSeal;
		s.text = Text;
		s.thisAction = action;

		undoHistory.push_back(s);

		// Reset the current undo history position to the end of the history
		undoPos = undoHistory.end();
	}
}

void TextBox::undo()
{
	// Validate undoPos
	if (undoHistory.empty())
		undoPos = undoHistory.end();

	// Decrement the history pointer to get the state to revert to
	// unless history pointer is already at the beginning of the undo history
	// or the undo history is empty

	if (undoHistory.empty())
		return;

	if (undoPos == ++undoHistory.begin() && undoHistory.size() > 1)
		return;

	// If we are at the end of the undo history (not undone anything),
	// save the current state in case the user wants to redo
	if (undoPos == undoHistory.end())
	{
		TextBoxState s;
		s.caret = Caret;
		s.charStart = charStart;
		s.seal = false;
		s.text = Text;
		s.thisAction = TBSA_Add;//arbitrary choice of action

		// Only save if it's not identical to last item in history
		// (which will be the case if we do undo, redo then undo again),
		// or if the history is empty
		--undoPos;

		if (undoPos->text != s.text)
		{
			++undoPos;
			undoHistory.push_back(s);
			--undoPos;
		}

		++undoPos;
	}

	// Point to item we want to set as current state
	--undoPos;
	--undoPos;

	// Overwrite the existing state with the history pointer state
	Caret = undoPos->caret;
	charStart = undoPos->charStart;
	Text = undoPos->text;

	// Unseal as we go backwards
	undoPos->seal = false;

	// Point to last undone item
	++undoPos;
}

void TextBox::redo()
{
	// Validate undoPos
	if (undoHistory.empty())
		undoPos = undoHistory.end();

	// Overwrite the existing state with the history pointer state
	// unless history pointer is already at the end of the undo history
	// or the undo history is empty

	if (undoHistory.empty())
		return;

	if (undoPos == undoHistory.end())
		return;

	Caret = undoPos->caret;
	charStart = undoPos->charStart;
	Text = undoPos->text;

	// Increment the history pointer
	++undoPos;
}

void TextBox::deleteSelection()
{
	if (selectionPoint != string::npos)
	{
		push(TBSA_DeleteBack, true);

		int selStart = min(selectionPoint, Caret);
		int selEnd = max(selectionPoint, Caret);
		Text.erase(selStart, selEnd - selStart);
		Caret = selStart;
		selectionPoint = string::npos;
	}
}

// Clickable button helper functions
void Button::doOnMouseUnhover(int, int, WPARAM)
{
	if (active)
		buttonBrush = noHoverBrush;
}

void Button::doOnMouseHover(int, int, WPARAM)
{
	if (active)
		buttonBrush = hoverBrush;
}

bool Button::doOnMouseButton(UINT button, int, int, WPARAM)
{
	if (active && button == WM_LBUTTONUP)
	{
		onClick(*this);
		return true;
	}
	return false;
}

void Button::doDraw()
{
	renderer->FillRoundedRectangleWH(x, y, w, h, rx, ry, buttonBrush);
	renderer->Text(x + (w - static_cast<int>(metrics.width)) / 2, y + (h - static_cast<int>(metrics.height)) / 2, text, textBrush, textFormat);
}

void Button::SetText(string text)
{
	this->text = text;
	TextLayout layout = renderer->MakeTextLayout(text, textFormat, renderer->ResolutionX, renderer->ResolutionY);
	layout->GetMetrics(&metrics);
}

// Draggable slider helper functions
int Slider::GetValue()
{
	return value;
}

int Slider::SetValue(int v)
{
	value = min(max(v, valueMin), valueMax);
	return value;
}

void Slider::update(int x)
{
	if (x < this->x || x >= this->x + this->w)
		return;

	float pc = static_cast<float>(x - this->x) / w;

	int previous = value;

	value = static_cast<int>(static_cast<float>((valueMax - valueMin) * pc) + 0.5) + valueMin;

	if (onChange && value != previous)
		onChange(*this);
}

bool Slider::doOnMouseButton(UINT button, int x, int y, WPARAM keys)
{
	if (button == WM_LBUTTONDOWN)
	{
		update(x);
		return true;
	}
	return false;
}

bool Slider::doOnMouseMove(int x, int y, WPARAM keys)
{
	// Drag action
	if (keys == MK_LBUTTON)
	{
		update(x);
		return true;
	}
	return false;
}

void Slider::doDraw()
{
	// Bar thickness
	int barHeight = 6;

	// Slider thickness
	int sliderWidth = 8;

	// Draw bar
	renderer->FillRoundedRectangleWH(x, y + (h - barHeight) / 2, w, barHeight, barHeight / 2, barHeight / 2, barBrush);

	// Draw slider
	renderer->FillRoundedRectangleWH(x + static_cast<int>((value - valueMin) / static_cast<float>((valueMax - valueMin)) * w) - sliderWidth / 2,
		y, sliderWidth, h, sliderWidth / 2, sliderWidth / 2, sliderBrush);
}

// Checkbox helper functions
void CheckBox::doDraw()
{
	// Corner curves
	int curveRadius = 2;

	// Horizontal gap between box and text
	int gap = 5;

	// Draw box
	renderer->DrawRoundedRectangleWH(x, y, w, h, curveRadius, curveRadius, boxBrush);

	// Draw a tick if needed
	if (checked)
	{
		renderer->Line(x + 2, y + h/2, x + w/2, y + h - 2, tickBrush, 2.0f);
		renderer->Line(x + w/2, y + h - 2, x + w - 2, y + 2, tickBrush, 2.0f);
	}

	// Draw text
	renderer->Text(x + w + gap, y + (h - static_cast<int>(metrics.height)) / 2, text, textBrush, textFormat);
}

bool CheckBox::doOnMouseButton(UINT button, int x, int y, WPARAM keys)
{
	if (button == WM_LBUTTONDOWN)
	{
		checked = !checked;
		if (onChange)
			onChange(*this);

		return true;
	}
	return false;
}

// Skinned objects
void Dialog::updateLayout()
{
	Clear();
	skin->ButtonSkin->GenerateObjects(ButtonBar);
	activeTextBoxes = skin->TextBoxSkin->GenerateObjects(TextBoxes);
	skin->HeaderLabel = TopLabel;
}

// End Scene namespace
}

// Utility functions

// WCHAR * -> std::string
string StringFactory(WCHAR *s)
{
	char *wn = new char[(wcslen(s)+1) * 2];
	size_t converted = 0;
	wcstombs_s(&converted, wn, (wcslen(s)+1) * 2, s, _TRUNCATE);

	string result(wn);
	delete [] wn;
	return result;
}

// std::string -> WCHAR * (allocates memory which should be freed by the app)
WCHAR *StringToWCHAR(string s)
{
	WCHAR *n = new WCHAR[s.length() + 1];
	size_t chars;
	mbstowcs_s(&chars, n, s.length() + 1, s.c_str(), _TRUNCATE);

	return n;
}

// Print debug string
void DebugPrint(std::string debugString)
{
#ifdef _DEBUG
	WCHAR *s = StringToWCHAR(debugString);
	OutputDebugStringW(s);
	delete [] s;
#endif
}

// Create MessageBox from string
void MessageBoxS(std::string debugString)
{
	WCHAR *s = StringToWCHAR(debugString);
	MessageBoxW(nullptr, s, nullptr, MB_OK);
	delete [] s;
}

// Constructor and destructor
Simple2D *Simple2D::App = nullptr;
std::mt19937 Simple2D::RandomEngine;
bool Simple2D::hasFocus = false;

// NOTE: Hopefully in Visual Studio 2013 we can just use Simple2D(Simple2DStartupInfo())
Simple2D::Simple2D() : Simple2D({
	"", "", "", 640, 480, 9.1, false, 0, "Simple2D Application", RB_Center,
	D2D1::ColorF(D2D1::ColorF::White), true, true, false, false, false, 0, 0, false, 0
}) {}

Simple2D::Simple2D(Simple2DStartupInfo si) :
	killbit(false),
	m_hwnd_app(NULL), vSyncClamp(si.VSyncClamp),
	resizeBehaviour(si.ResizeBehaviourType),
	Direct2D(NULL), TextFactory(NULL), ImageFactory(NULL),
	Direct3D(NULL), Screen3D(NULL), Direct2DDevice(NULL),
	DXGISwapChain(NULL), DXGISwapChainWindowed(NULL), DXGISwapChainFullScreen(NULL),
	Direct2DBackBuffer(NULL),
	Screen(NULL),
	WindowName(NULL),
	MinWindowSizeX(si.MinimumWindowSizeX), MinWindowSizeY(si.MinimumWindowSizeY),
	ResolutionX(si.ResolutionX), ResolutionY(si.ResolutionY),
	ClientW(si.ResolutionX), ClientH(si.ResolutionY),
	RenderTargetX(0), RenderTargetY(0),
	RenderTargetW(si.ResolutionX), RenderTargetH(si.ResolutionY),
	BackgroundColour(si.BackgroundColour),
	CurrentBrush(NULL), CurrentTextFormat(NULL),
	EnableClear(si.EnableClear), EnableFullScreen(si.EnableFullScreen),
	FullScreen(si.FullScreen),
	EnableModeSwitch(si.EnableModeSwitch), ResizableWindow(si.ResizableWindow),
	ShowFps(false), DataPath(""),
	Enable3D(si.Enable3D), EnableMSAA(si.EnableMSAA),
	scene(NULL), renderSceneAfter(false),
	focusObject(NULL)
{
	// Set app pointer
	Simple2D::App = this;

	// Randomize
	srand(static_cast<unsigned int>(time(NULL)));
	RandomEngine.seed(static_cast<unsigned int>(time(NULL)));

	// Default settings
	QueryPerformanceCounter((LARGE_INTEGER *) &LastUpdateTime64);
	QueryPerformanceFrequency((LARGE_INTEGER *) &PerformanceFrequency);

	// Resolve application storage path
	if (si.ManufacturerName != "" && si.AppName != "" && si.Version != "")
	{
		WCHAR *path;
		SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
		DataPath = StringFactory(path) + "\\" + si.ManufacturerName + "\\" + si.AppName + "\\" + si.Version + "\\";

		// Create the path (including all sub-directories) if it doesn't already exist
		SHCreateDirectoryEx(NULL, DataPath.c_str(), NULL);

		CoTaskMemFree(path);
	}

	// Configure heap
	// Use HeapSetInformation to specify that the process should
	// terminate if the heap manager detects an error in any heap used
	// by the process.
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// Initialize COM
	CoInitialize(NULL);

	// Set minimum Direct3D feature level
	int major = static_cast<int>(si.MinimumFeatureLevel);
	int minor = static_cast<int>((si.MinimumFeatureLevel - floor(si.MinimumFeatureLevel)) * 10.f);

	minFeatureLevel = static_cast<D3D_FEATURE_LEVEL>(major * 0x1000 + minor * 0x0100);

	// Initialize the application
	HRSilentDieOnFail(Initialize(si.WindowName, si.IconResource));

	// Create overlay scene
	Overlay.reset(new Scene());
}

Simple2D::~Simple2D()
{
	ReleaseInitialResources();

	SafeRelease(&DXGISwapChain);
	SafeRelease(&Direct2DDevice);
	SafeRelease(&Screen3D);
	SafeRelease(&Direct3D);

	SafeRelease(&ImageFactory);
	CoUninitialize();

	SafeRelease(&TextFactory);
	SafeRelease(&Direct2D);

	if (WindowName)
		delete [] WindowName;
}

// Entry point / Message loop
void Simple2D::Run()
{
	// Set up one-time application-specific resources
	if (!SetupInitialResources())
		HRDieOnFail(
			E_FAIL,
			"There was a problem initializing the application's resources");

	// Initialize application graphics
	// This must come before we display the window otherwise
	// CreateDeviceResources() could be called from WndProc() before
	// we get chance to finish here!
	// Also calls SetupResources()
	HRSilentDieOnFail(CreateDeviceResources());

	// Initialize application data
	SetupApplication();

	// Display window
	ShowWindow(m_hwnd_app, SW_SHOWNORMAL);
	ShowWindow(m_hwnd_rt_windowed, SW_SHOWNORMAL);
	UpdateWindow(m_hwnd_app);

	// Windows message pump
	MSG msg;

	while (!killbit)
	{
		BOOL process;

		// Stall the message loop if the window is minimized to save CPU time (GetMessage is a blocking function)
		if (!IsMinimized(m_hwnd_app))
			process = PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
		else
			process = GetMessage(&msg, NULL, 0, 0);

		if (process)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!killbit)
			onRender();
	}
}

// Initialize creates the window, shows it and calls CreateDeviceIndependentResources
// then CreateDeviceResources and SetupApplication
HRESULT Simple2D::Initialize(string windowName, int iconResource)
{
	// Register the window class
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= Simple2D::WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= HINST_THISCOMPONENT;
	if (iconResource)
		wcex.hIcon		= LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(iconResource));
	wcex.hbrBackground	= (HBRUSH) GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName	= "Simple2DApplication";

	RegisterClassEx(&wcex);

	// Define window name
	WindowName = StringToWCHAR(windowName);

	char *wn = new char[(wcslen(WindowName)+1) * 2];
	size_t converted = 0;
	wcstombs_s(&converted, wn, (wcslen(WindowName)+1) * 2, WindowName, _TRUNCATE);

	// Create the window
	m_hwnd_app = CreateWindow(
		"Simple2DApplication", wn,
			(ResizableWindow?
			WS_OVERLAPPEDWINDOW :
			WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX)
				| WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, NULL, NULL, HINST_THISCOMPONENT, this);

	delete [] wn;

	if (!m_hwnd_app)
		HRReturnOnFail(
			E_FAIL,
			"There was a problem creating the application window");

	// Create render target sub-window
	// We use this to cheat with resizing behaviour
	m_hwnd_rt_windowed = CreateWindow("STATIC", "",
			WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			0, 0, ResolutionX, ResolutionY, m_hwnd_app, NULL, HINST_THISCOMPONENT, this);

	// Create full-screen window if needed
	if (EnableFullScreen)
	{
		m_hwnd_rt_fullscreen = CreateWindow("Simple2DApplication", "", WS_OVERLAPPEDWINDOW,
				0, 0, ResolutionX, ResolutionY, NULL, NULL, HINST_THISCOMPONENT, this);
	}
	else
		m_hwnd_rt_fullscreen = NULL;

	// Initialize device-independent resources, such as the Direct2D factory
	// This must be done after the windows are created because the HWNDs are needed
	// to create DXGISwapChains, but we need Direct2D to get the desktop DPI so it
	// must be called before the window sizes are set
	HRSilentReturnOnFail(CreateDeviceIndependentResources());

	// Obtain the system DPI and use it to scale the window size
	FLOAT dpiX, dpiY;

	// The factory returns the current system DPI. This is also the value it will use
	// to create its own windows.
	Direct2D->GetDesktopDpi(&dpiX, &dpiY);

	UINT w = static_cast<UINT>(ceil(static_cast<float>(ResolutionX) * dpiX / 96.f));
	UINT h = static_cast<UINT>(ceil(static_cast<float>(ResolutionY) * dpiY / 96.f));

	// Resize the window to take into account the border width and height
	RECT rcClient, rcWindow;
	POINT borderSize;
	GetClientRect(m_hwnd_app, &rcClient);
	GetWindowRect(m_hwnd_app, &rcWindow);
	borderSize.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	borderSize.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(m_hwnd_app, rcWindow.left, rcWindow.top, w + borderSize.x, h + borderSize.y, FALSE);

	ClientW = ResolutionX;
	ClientH = ResolutionY;
	RenderTargetX = 0;
	RenderTargetY = 0;
	RenderTargetW = ResolutionX;
	RenderTargetH = ResolutionY;

	return S_OK;
}

// Create Direct2D resources
// Device-independent resources last for the lifetime of the application
// Device-dependent resources are associated with a particular rendering device and will cease to function if that device is removed
HRESULT Simple2D::CreateDeviceIndependentResources()
{
	// Create a Direct2D factory
	if (!Direct2D)
	{
		D2D1_FACTORY_OPTIONS options;
		ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

		HRReturnOnFail(
			D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, reinterpret_cast<void **>(&Direct2D)),
			"There was a problem setting up the Direct2D factory");
	}

	// Create a DirectWrite factory
	if (!TextFactory)
		HRReturnOnFail(
			DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(TextFactory),
									reinterpret_cast<IUnknown **>(&TextFactory)),
			"There was a problem setting up the DirectWrite factory");

	// Create a Windows Imaging factory
	if (!ImageFactory)
		HRReturnOnFail(
			CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ImageFactory)),
			"There was a problem setting up the WIC Imaging factory");

	// =================================================================================
	// Direct2D 1.1 version + Direct3D support
	// =================================================================================

	// This flag adds support for surfaces with a different color channel ordering 
	// than the API default. It is required for compatibility with Direct2D. 
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; 

	// Set feature levels supported by our application
	D3D_FEATURE_LEVEL featureLevels[] = 
	{ 
		D3D_FEATURE_LEVEL_11_1, 
		D3D_FEATURE_LEVEL_11_0, 
		D3D_FEATURE_LEVEL_10_1, 
		D3D_FEATURE_LEVEL_10_0, 
		D3D_FEATURE_LEVEL_9_3, 
		D3D_FEATURE_LEVEL_9_2, 
		D3D_FEATURE_LEVEL_9_1 
	};

	// Create Direct3D device and context
	ID3D11Device *device;
	ID3D11DeviceContext *context;
	D3D_FEATURE_LEVEL returnedFeatureLevel;
		
	HRReturnOnFail(
		D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags,
				featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
				&device, &returnedFeatureLevel, &context),
		"Could not create the Direct3D device");

	// Direct3D-specific: Check for minimum feature level support
	if (Enable3D)
		if (returnedFeatureLevel < minFeatureLevel)
		{
			SafeRelease(&context);
			SafeRelease(&device);

			HRReturnOnFail(
				E_FAIL,
				"Your graphics card does not support the minimum requirements for this application");
		}

	// Get the underlying versioned device and context interfaces
	device->QueryInterface(__uuidof(ID3D11Device1), (void **)&Direct3D);
	context->QueryInterface(__uuidof(ID3D11DeviceContext1), (void **)&Screen3D);

	// Clean up temporary references
	SafeRelease(&context);
	SafeRelease(&device);

	// Get MSAA quality
	UINT msaaQuality;

	if (Enable3D)
	{
		HRReturnOnFail(
			Direct3D->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, 4, &msaaQuality),
			"Could not query graphics card for multi-sampling/anti-aliasing support");

		if (msaaQuality <= 0)
			HRReturnOnFail(
				E_FAIL,
				"Your graphics card does not support 4x MSAA");
	}

	// Get the underlying DXGI device of the Direct3D device
	IDXGIDevice *dxgiDevice;
	Direct3D->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);

	// Create 2D device object
	HRReturnOnFail(
		Direct2D->CreateDevice(dxgiDevice, &Direct2DDevice),
		"Could not create the Direct2D device");

	// Get the GPU we are using
	IDXGIAdapter *dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);

	// Get the DXGI factory instance
	IDXGIFactory2 *dxgiFactory;
	dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

	// Describe Windows 7-compatible Windowed swap chain (DXGI_SCALING_NONE is not allowed in Windows 7)
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo = false;

	if (Enable3D && EnableMSAA)
	{
		swapChainDesc.SampleDesc.Count = 4;
		swapChainDesc.SampleDesc.Quality = msaaQuality - 1;
	}
	else
	{
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = EnableModeSwitch? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

	// Create DXGI swap chain targeting a window handle (the only Windows 7-compatible option)
	HRReturnOnFail(
		dxgiFactory->CreateSwapChainForHwnd(Direct3D, m_hwnd_rt_windowed, &swapChainDesc, nullptr, nullptr, &DXGISwapChainWindowed),
		"Could not create windowed swap chain");

	dxgiFactory->MakeWindowAssociation(m_hwnd_rt_windowed, DXGI_MWA_NO_ALT_ENTER);

	// Create DXGI swap chain for full-screen mode if needed
	if (EnableFullScreen)
	{
		HRReturnOnFail(
			dxgiFactory->CreateSwapChainForHwnd(Direct3D, m_hwnd_rt_fullscreen, &swapChainDesc, nullptr, nullptr, &DXGISwapChainFullScreen),
			"Could not create full-screen swap chain");

		// Disable Alt+Enter on this window - we'll deal with it ourselves in the window message handler
		// If we don't use NO_WINDOW_CHANGES here, we don't receive WM_MENUCHAR messages in full-screen mode
		dxgiFactory->MakeWindowAssociation(m_hwnd_rt_fullscreen, DXGI_MWA_NO_WINDOW_CHANGES);

		// Make full-screen if we are starting in full-screen mode
		// NOTE: This will change the display resolution
		if (FullScreen)
		{
			// The input focus will screw up if we don't include this line
			ShowWindow(m_hwnd_rt_fullscreen, SW_SHOWNORMAL);

			DXGISwapChainFullScreen->SetFullscreenState(TRUE, NULL);

			// Get monitor
			IDXGIOutput *mainOutput;
			DXGISwapChainFullScreen->GetContainingOutput(&mainOutput);
			
			// Get desktop resolution of monitor
			DXGI_OUTPUT_DESC desc;
			mainOutput->GetDesc(&desc);
						
			// Set informational variables
			RenderTargetW = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
			RenderTargetH = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;

			// Release monitor
			SafeRelease(&mainOutput);
		}
	}

	// Active swap chain depends on whether we start windowed or full-screen
	DXGISwapChain = (FullScreen && EnableFullScreen? DXGISwapChainFullScreen : DXGISwapChainWindowed);

	// Clean up
	SafeRelease(&dxgiFactory);
	SafeRelease(&dxgiAdapter);
	SafeRelease(&dxgiDevice);

	return S_OK;
}

// Create a render target and device-specific resources
HRESULT Simple2D::CreateDeviceResources()
{
	// =================================================================================
	// Direct2D 1.1 version
	// =================================================================================

	// Window-size-dependent resources
	SafeRelease(&Direct2DBackBuffer);
	SafeRelease(&Screen);

	// Create 2D device context (replaces ID2D1RenderTarget sub-classes)
	Direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &Screen);

	// Get the back buffer as an IDXGISurface (Direct2D doesn't accept an ID3D11Texture2D directly as a render target)
	IDXGISurface *dxgiBackBuffer;
	DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));

	// Get screen DPI
	FLOAT dpiX, dpiY;
	Direct2D->GetDesktopDpi(&dpiX, &dpiY);

	// Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpiX, dpiY);

	Screen->CreateBitmapFromDxgiSurface(dxgiBackBuffer, &bitmapProperties, &Direct2DBackBuffer);

	// Set surface as render target in Direct2D device context
	Screen->SetTarget(Direct2DBackBuffer);

	// Direct3D-specific initialization
	if (Enable3D)
		HRSilentReturnOnFail(CreateDirect3DResources());

	// Clean up
	SafeRelease(&dxgiBackBuffer);

	// =================================================================================
	// End of Direct2D 1.1 version
	// =================================================================================

	// Use render target to create resources
	if (!SetupResources())
		HRReturnOnFail(
			E_FAIL,
			"Could not set up the application resources");

	return S_OK;
}

// Create Direct3D-specific device resources
HRESULT Simple2D::CreateDirect3DResources()
{
	// Get MSAA quality
	UINT msaaQuality;
	Direct3D->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, 4, &msaaQuality);

	// Direct3D-specific: Get the back buffer as an ID3D11Resource
	ID3D11Resource *dxgiBackBufferAsTexture;
	DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBufferAsTexture));

	// Direct3D-specific: Create a view of the render target for the 3D rendering pipeline
	HRReturnOnFail(
		Direct3D->CreateRenderTargetView(dxgiBackBufferAsTexture, NULL, &RenderTargetView),
		"Could not create Direct3D render target view");

	// Direct3D-specific: Create depth/stencil buffer description
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width = ResolutionX;
	dsDesc.Height = ResolutionY;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (EnableMSAA)
	{
		dsDesc.SampleDesc.Count = 4;
		dsDesc.SampleDesc.Quality = msaaQuality - 1;
	}
	else
	{
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
	}

	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;

	// Direct3D-specific: Create depth/stencil buffer and view
	HRReturnOnFail(
		Direct3D->CreateTexture2D(&dsDesc, NULL, &depthStencilBuffer),
		"Could not create depth/stencil buffer");

	HRReturnOnFail(
		Direct3D->CreateDepthStencilView(depthStencilBuffer, NULL, &DepthStencilView),
		"Could not create depth/stencil view");

	// Direct3D-specific: Bind views to Output Merger stage
	Screen3D->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);

	// Direct3D-specific: Set viewport to entire client area
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(ResolutionX);
	viewport.Height = static_cast<float>(ResolutionY);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	Screen3D->RSSetViewports(1, &viewport);

	// Clean up
	SafeRelease(&dxgiBackBufferAsTexture);

	return S_OK;
}

// Free Direct3D-specific device resources
void Simple2D::DiscardDirect3DResources()
{
	// Direct3D
	if (Enable3D)
	{
		SafeRelease(&DepthStencilView);
		SafeRelease(&depthStencilBuffer);
		SafeRelease(&RenderTargetView);

		Screen3D->OMSetRenderTargets(1, &RenderTargetView, NULL);
	}
}

// Free device-dependent resources
void Simple2D::DiscardDeviceResources()
{
	// Invalidate all device-dependent rendering resources
	for (auto i = RenderingObjects.begin(); i != RenderingObjects.end(); i++)
		i->second->Invalidate();

	RenderingObjects.clear();

	// Release application resources
	ReleaseResources();

	// Direct3D
	DiscardDirect3DResources();

	// Direct2D 1.1
	SafeRelease(&Direct2DBackBuffer);
	SafeRelease(&Screen);			// Direct2D 1.0 also (this line only)
}

// Render Direct2D content
LRESULT CALLBACK Simple2D::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		Simple2D *pSimple2D = (Simple2D *)pcs->lpCreateParams;

		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pSimple2D));

		result = 1;
	}

	else
	{
		Simple2D *pSimple2D = reinterpret_cast<Simple2D *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));

		bool wasHandled = false;

		if (pSimple2D)
		{
			if (pSimple2D->focusObject)
				if (pSimple2D->focusObject->OnWindowsMessage(hwnd, message, wParam, lParam))
					return 0;

			if (pSimple2D->OnWindowsMessage(hwnd, message, wParam, lParam))
				return 0;

			switch (message)
			{
				// Resize window
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					pSimple2D->OnWindowResize(width, height);
					InvalidateRect(hwnd, NULL, true);
				}
				result = 0;
				wasHandled = true;
				break;

				// Window moved over our window
			case WM_DISPLAYCHANGE:
				{
					InvalidateRect(hwnd, NULL, true);
				}
				result = 0;
				wasHandled = true;
				break;

				// Focus changes
			case WM_SETFOCUS:
				{
					hasFocus = true;
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_KILLFOCUS:
				{
					hasFocus = false;
				}
				result = 0;
				wasHandled = true;
				break;

				// Minimum window size enforcement
			case WM_GETMINMAXINFO:
				((MINMAXINFO*)lParam)->ptMinTrackSize.x = pSimple2D->MinWindowSizeX;
				((MINMAXINFO*)lParam)->ptMinTrackSize.y = pSimple2D->MinWindowSizeY;
				result = 0;
				wasHandled = true;
				break;

				// Handle switch between windowed and full-screen mode via Alt+Enter
			case WM_MENUCHAR:
				if (LOWORD(wParam) == VK_RETURN)
				{
					// Switch to full-screen
					if (pSimple2D->DXGISwapChain == pSimple2D->DXGISwapChainWindowed && pSimple2D->EnableFullScreen)
					{
						// Set active swap chain
						pSimple2D->DXGISwapChain = pSimple2D->DXGISwapChainFullScreen;

						// Display full-screen render target window
						ShowWindow(pSimple2D->m_hwnd_rt_fullscreen, SW_NORMAL);

						// Activate full-screen mode in swap chain
						pSimple2D->DXGISwapChain->SetFullscreenState(TRUE, NULL);
					}

					// Switch to windowed
					else if (pSimple2D->DXGISwapChain == pSimple2D->DXGISwapChainFullScreen)
					{
						// De-activate full-screen mode in swap chain
						pSimple2D->DXGISwapChain->SetFullscreenState(FALSE, NULL);

						// Hide full-screen render target window
						ShowWindow(pSimple2D->m_hwnd_rt_fullscreen, SW_HIDE);

						// Set active swap chain
						pSimple2D->DXGISwapChain = pSimple2D->DXGISwapChainWindowed;

						ResizeBehaviour rbPrev = pSimple2D->resizeBehaviour;
						int cw = pSimple2D->ClientW, ch = pSimple2D->ClientH;

						RECT rcClient;
						GetClientRect(pSimple2D->m_hwnd_app, &rcClient);

						// Force resource re-creation
						pSimple2D->resizeBehaviour = RB_Resize;
						pSimple2D->OnWindowResize(pSimple2D->ResolutionX, pSimple2D->ResolutionY);

						// Set window and render target sizes and re-size behaviour to previous settings
						pSimple2D->resizeBehaviour = rbPrev;
						pSimple2D->OnWindowResize(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

						// Give keyboard input focus to application window
						SetFocus(pSimple2D->m_hwnd_app);
					}

					// Supress unknown accelerator key beep
					result = MAKELRESULT(0, MNC_CLOSE);
					wasHandled = true;
				}
				break;

				// Key press
			case WM_CHAR:
				{
					if (pSimple2D->focusObject)
						wasHandled = pSimple2D->focusObject->OnKeyCharacter(static_cast<int>(wParam), LOWORD(lParam), static_cast<bool>((lParam >> 30) & 1), (lParam >> 31) == 1);

					if (!wasHandled)
						if (pSimple2D->scene)
							wasHandled = pSimple2D->scene->OnKeyCharacter(static_cast<int>(wParam), LOWORD(lParam), static_cast<bool>((lParam >> 30) & 1), (lParam >> 31) == 1);

					if (!wasHandled)
						wasHandled = pSimple2D->Overlay->OnKeyCharacter(static_cast<int>(wParam), LOWORD(lParam), static_cast<bool>((lParam >> 30) & 1), (lParam >> 31) == 1);

					if (!wasHandled)
						wasHandled = pSimple2D->OnKeyCharacter(static_cast<int>(wParam), LOWORD(lParam), static_cast<bool>((lParam >> 30) & 1), (lParam >> 31) == 1);
				}
				result = 0;
				wasHandled = true;
				break;

				// Special key press
			case WM_KEYDOWN:
				{
					if (pSimple2D->focusObject)
						wasHandled = pSimple2D->focusObject->OnKeyDown(static_cast<int>(wParam), LOWORD(lParam), static_cast<bool>((lParam >> 30) & 1));

					if (!wasHandled)
						if (pSimple2D->scene)
							wasHandled = pSimple2D->scene->OnKeyDown(static_cast<int>(wParam), LOWORD(lParam), static_cast<bool>((lParam >> 30) & 1));

					if (!wasHandled)
						wasHandled = pSimple2D->Overlay->OnKeyDown(static_cast<int>(wParam), LOWORD(lParam), static_cast<bool>((lParam >> 30) & 1));

					if (!wasHandled)
						wasHandled = pSimple2D->OnKeyDown(static_cast<int>(wParam), LOWORD(lParam), static_cast<bool>((lParam >> 30) & 1));
				}
				result = 0;
				wasHandled = true;
				break;

				// Special key press
			case WM_KEYUP:
				{
					if (pSimple2D->focusObject)
						wasHandled = pSimple2D->focusObject->OnKeyUp(static_cast<int>(wParam), LOWORD(lParam));

					if (!wasHandled)
						if (pSimple2D->scene)
							wasHandled = pSimple2D->scene->OnKeyUp(static_cast<int>(wParam), LOWORD(lParam));

					if (!wasHandled)
						wasHandled = pSimple2D->Overlay->OnKeyUp(static_cast<int>(wParam), LOWORD(lParam));

					if (!wasHandled)
						wasHandled = pSimple2D->OnKeyUp(static_cast<int>(wParam), LOWORD(lParam));
				}
				result = 0;
				wasHandled = true;
				break;

				// Mouse movement
			case WM_MOUSEMOVE:
				{
					// Get absolute mouse co-ordinates relative to top-left corner of application window
					int x = GET_X_LPARAM(lParam);
					int y = GET_Y_LPARAM(lParam);

					// Convert them to co-ordinates relative to the render target child window
					x -= pSimple2D->RenderTargetX;
					y -= pSimple2D->RenderTargetY;
					x = static_cast<int>(static_cast<float>(x) / (static_cast<float>(pSimple2D->RenderTargetW) / pSimple2D->ResolutionX));
					y = static_cast<int>(static_cast<float>(y) / (static_cast<float>(pSimple2D->RenderTargetH) / pSimple2D->ResolutionY));

					if (pSimple2D->scene)
						wasHandled = pSimple2D->scene->OnMouseMove(x, y, wParam);

					if (!wasHandled)
						wasHandled = pSimple2D->Overlay->OnMouseMove(x, y, wParam);

					if (!wasHandled)
						wasHandled = pSimple2D->OnMouseMove(x, y, wParam);
				}
				result = 0;
				wasHandled = true;
				break;

				// Mouse clicks
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
				{
					// Clear focus object if left-click
					if (message == WM_LBUTTONDOWN)
						if (pSimple2D->focusObject)
						{
							pSimple2D->focusObject->SetFocus(false);
							pSimple2D->focusObject = nullptr;
						}

					// Get absolute mouse co-ordinates relative to top-left corner of application window
					int x = GET_X_LPARAM(lParam);
					int y = GET_Y_LPARAM(lParam);

					// Convert them to co-ordinates relative to the render target child window
					x -= pSimple2D->RenderTargetX;
					y -= pSimple2D->RenderTargetY;
					x = static_cast<int>(static_cast<float>(x) / (static_cast<float>(pSimple2D->RenderTargetW) / pSimple2D->ResolutionX));
					y = static_cast<int>(static_cast<float>(y) / (static_cast<float>(pSimple2D->RenderTargetH) / pSimple2D->ResolutionY));

					if (pSimple2D->scene)
						wasHandled = pSimple2D->scene->OnMouseButton(message, x, y, wParam);

					if (!wasHandled)
						wasHandled = pSimple2D->Overlay->OnMouseButton(message, x, y, wParam);

					if (!wasHandled)
						wasHandled = pSimple2D->OnMouseButton(message, x, y, wParam);
				}
				result = 0;
				wasHandled = true;
				break;

				// Render window
			case WM_PAINT:
				{
					// We need this in case there are (RB_Zoom / RB_Center) borders on the application window
					// They won't get re-drawn properly if we don't clear the background ourselves
					PAINTSTRUCT ps;
					HDC hdc = BeginPaint(hwnd, &ps);
					RECT rc;
					GetClientRect(hwnd, &rc);
					FillRect(hdc, &rc, (HBRUSH) GetStockObject(BLACK_BRUSH));
					EndPaint(hwnd, &ps);

					pSimple2D->onRender();
					ValidateRect(hwnd, NULL);
				}
				result = 0;
				wasHandled = true;
				break;

				// Close window
			case WM_DESTROY:
				{
					PostQuitMessage(0);
					pSimple2D->killbit = true;
					pSimple2D->DiscardDeviceResources();
				}
				result = 1;
				wasHandled = true;
				break;
			}
		}

		// Forward all unhandled window messages to default window handler
		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
	
	return result;
}


// Resize the render target when the window is resized
void Simple2D::OnWindowResize(UINT width, UINT height)
{
	// Prevent re-sizing the render target in full-screen (forces either stretch or a mode switch)
	if (DXGISwapChain == DXGISwapChainFullScreen)
	{
		width = ResolutionX;
		height = ResolutionY;
	}

	// Update size of application window
	ClientW = width;
	ClientH = height;

	if (DXGISwapChain == DXGISwapChainWindowed)
	{
		// Stretch the render target to match the window client area
		if (resizeBehaviour == ResizeBehaviour::RB_Stretch)
		{
			RenderTargetX = RenderTargetY = 0;
			RenderTargetW = ClientW;
			RenderTargetH = ClientH;
			MoveWindow(m_hwnd_rt_windowed, RenderTargetX, RenderTargetY, RenderTargetW, RenderTargetH, false);
		}

		// Center the render target window
		if (resizeBehaviour == ResizeBehaviour::RB_Center)
		{
			RenderTargetX = (ClientW - ResolutionX) / 2;
			RenderTargetY = (ClientH - ResolutionY) / 2;
			RenderTargetW = ResolutionX;
			RenderTargetH = ResolutionY;
			MoveWindow(m_hwnd_rt_windowed, RenderTargetX, RenderTargetY, RenderTargetW, RenderTargetH, false);
		}

		// Zoom the render target window while maintaining aspect ratio
		if (resizeBehaviour == ResizeBehaviour::RB_Zoom)
		{
			float aspectRatio = static_cast<float>(ResolutionX) / ResolutionY;

			float mult = min(static_cast<float>(ClientW) / ResolutionX, static_cast<float>(ClientH) / ResolutionY);

			RenderTargetX = static_cast<int>((ClientW - static_cast<float>(ResolutionX) * mult) / 2);
			RenderTargetY = static_cast<int>((ClientH - static_cast<float>(ResolutionY) * mult) / 2);
			RenderTargetW = static_cast<int>(ResolutionX * mult);
			RenderTargetH = static_cast<int>(ResolutionY * mult);
			MoveWindow(m_hwnd_rt_windowed, RenderTargetX, RenderTargetY, RenderTargetW, RenderTargetH, false);
		}
	}

	// Resize the render target to match the window client area
	// Note that full-screen render targets always behave as if ResizeBehaviour == RB_Resize
	// (the resize is only done when switching to or from a full-screen display)
	if ((resizeBehaviour == ResizeBehaviour::RB_Resize && DXGISwapChain == DXGISwapChainWindowed)
		|| DXGISwapChain == DXGISwapChainFullScreen)
	{
		// Release Direct3D resources
		DiscardDirect3DResources();

		// Release Direct2D resources
		SafeRelease(&Direct2DBackBuffer);
		SafeRelease(&Screen);

		// Resize the render target child window to make it match the application window, if in windowed mode
		if (DXGISwapChain == DXGISwapChainWindowed)
			MoveWindow(m_hwnd_rt_windowed, 0, 0, width, height, false);

		// Resize render target buffers
		HRDieOnFail(
			DXGISwapChain->ResizeBuffers(0, ClientW, ClientH, DXGI_FORMAT_UNKNOWN, 0),
			"Could not resize render target");

		// Update resolution for application use
		ResolutionX = RenderTargetW = ClientW;
		ResolutionY = RenderTargetH = ClientH;
		RenderTargetX = RenderTargetY = 0;

		// Re-create Direct2D and Direct3D resources
		// (must be done after ResolutionX and ResolutionY are updated)
		CreateDeviceResources();

		// In full-screen mode, the render target might be stretched,
		// so take the resolution of the monitor in full-screen mode
		if (DXGISwapChain == DXGISwapChainFullScreen)
		{
			// Get monitor
			IDXGIOutput *mainOutput;
			DXGISwapChain->GetContainingOutput(&mainOutput);
			
			// Get desktop resolution of monitor
			DXGI_OUTPUT_DESC desc;
			mainOutput->GetDesc(&desc);
						
			// Set informational variables
			RenderTargetW = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
			RenderTargetH = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;

			// Release monitor
			SafeRelease(&mainOutput);
		}

		// Call application-specific OnResize fucntion
		OnResize(ResolutionX, ResolutionY);
	}
}

// Render the actual content
HRESULT Simple2D::onRender()
{
	// Re-create device resources if they have been destroyed
	// since the last time the window was painted
	if (!Screen)
		HRSilentReturnOnFail(CreateDeviceResources());

	// Retrieve size of drawing area and update stored resolution
	D2D1_SIZE_F rtSize = Screen->GetSize();
	ResolutionX = static_cast<int>(rtSize.width);
	ResolutionY = static_cast<int>(rtSize.height);

	// Don't do any rendering if window is minimized
	if (IsMinimized(m_hwnd_app))
		return S_OK;

	// Frame rate statistics
	static int frameCount = 0;
	static int ticksElapsed = GetTickCount();

	// Calculate previous frame time
	__int64 t;
	QueryPerformanceCounter((LARGE_INTEGER *) &t);
	LastFrameTime = t - LastUpdateTime64;
	LastUpdateTime64 = t;

	// Update physics
	UpdateObjects();

	// Start drawing
	Screen->BeginDraw();

	// Set transform to identity matrix
	Screen->SetTransform(D2D1::Matrix3x2F::Identity());

	// Clear window
	if (EnableClear)
		Screen->Clear(BackgroundColour);

	if (Enable3D)
		Screen->EndDraw();

	// Draw registered scene
	if (scene && !renderSceneAfter)
	{
		Scene *curScene = scene;

		scene->Update();
		curScene->Draw();
	}

	// Draw immediate scene
	DrawScene();

	// Draw registered scene
	if (scene && renderSceneAfter)
	{
		Scene *curScene = scene;

		scene->Update();
		curScene->Draw();
	}

	// Draw overlay scene
	Overlay->Update();
	Overlay->Draw();

	// Finish drawing
	if (!Enable3D)
	{
		HRESULT hr = Screen->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			DiscardDeviceResources();
			return S_OK;
		}
		else if (FAILED(hr))
			return hr;
	}

	// Present (new for Direct2D 1.1)
	DXGI_PRESENT_PARAMETERS parameters = { 0 };
	parameters.DirtyRectsCount = 0;
	parameters.pDirtyRects = nullptr;
	parameters.pScrollRect = nullptr;
	parameters.pScrollOffset = nullptr;

	DXGISwapChain->Present1(vSyncClamp, 0, &parameters);

	if (ShowFps)
	{
		frameCount++;

		if (GetTickCount() - ticksElapsed >= 1000)
		{
			ShowFPS(frameCount);
			ticksElapsed += 1000;
			frameCount = 0;
		}
	}

	return S_OK;
}

// Utility functions
void Simple2D::SetResizeBehaviour(ResizeBehaviour rb)
{
	resizeBehaviour = rb;
}

void Simple2D::SetBackgroundColour(D2D1_COLOR_F &col)
{
	BackgroundColour = col;
}

void Simple2D::SetBackgroundColour(D2D1::ColorF::Enum col)
{
	SetBackgroundColour(D2D1::ColorF(col));
}

void Simple2D::SetEnableClear(bool enable)
{
	EnableClear = enable;
}

void Simple2D::SetShowFps(bool enable)
{
	ShowFps = enable;
}

void Simple2D::SetRenderSceneAfter(bool after)
{
	renderSceneAfter = after;
}

void Simple2D::SetVSyncClamp(int clamp)
{
	vSyncClamp = clamp;
}

bool Simple2D::HasFocus()
{
	return Simple2D::hasFocus;
}

HWND Simple2D::GetWindow()
{
	return m_hwnd_app;
}

float Simple2D::GetAspectRatio() const
{
	return static_cast<float>(ResolutionX) / ResolutionY;
}

void Simple2D::SetScene(Scene &s, bool resetAnimations)
{
	if (scene)
		scene->OnDeactivate();

	scene = &s;

	if (resetAnimations)
		scene->ResetAnimations();

	scene->OnActivate();

	focusObject = nullptr;
}

Scene *Simple2D::GetScene()
{
	return scene;
}

void Simple2D::ClearScene()
{
	if (scene)
		scene->OnDeactivate();

	scene = NULL;

	focusObject = nullptr;
}

Scene &Simple2D::GetOverlay()
{
	return *Overlay.get();
}

void Simple2D::SetFocusObject(InterfaceObject &focus, bool set)
{
	SetFocusObject(&focus, set);
}

void Simple2D::SetFocusObject(InterfaceObject *focus, bool set)
{
	if (set)
		focusObject = focus;
	else
		if (focusObject == focus)
			focusObject = nullptr;
}

InterfaceObject *Simple2D::GetFocusObject()
{
	return focusObject;
}

// Get time in seconds since last frame
float Simple2D::GetLastFrameTime() const
{
	return static_cast<float>(LastFrameTime) / PerformanceFrequency;
}

// Animation helpers
float Simple2D::LinearMovement(float pixelsPerSecond) const
{
	return GetLastFrameTime() * pixelsPerSecond;
}

// Create managed resources
RenderingObject::RenderingObject() : renderer(Simple2D::App), resource(NULL), hash(0) {}
RenderingObject::RenderingObject(ID2D1Resource *res) : renderer(Simple2D::App), resource(res, false), hash(0) {}

PaintbrushObject::PaintbrushObject(ID2D1SolidColorBrush *b) : GenericBrush(b) {}
PaintbrushObject::PaintbrushObject(D2D1_COLOR_F &col) : GenericBrush(), colour(col) {}
PaintbrushObject::PaintbrushObject(D2D1::ColorF::Enum col) : GenericBrush(), colour(D2D1::ColorF(col)) {}

GradientObject::GradientObject(ID2D1LinearGradientBrush *b, AlignmentType gt, D2D1_EXTEND_MODE extendMode)
	: GradientObject(b, D2D1::ColorF(Colour::Black), D2D1::ColorF(Colour::Black), gt, extendMode) {}
GradientObject::GradientObject(D2D1_COLOR_F &start, D2D1_COLOR_F &end, AlignmentType gt, D2D1_EXTEND_MODE extendMode)
	: GradientObject((ID2D1LinearGradientBrush *)NULL, start, end, gt, extendMode) {}
GradientObject::GradientObject(D2D1::ColorF::Enum start, D2D1_COLOR_F &end, AlignmentType gt, D2D1_EXTEND_MODE extendMode)
	: GradientObject((ID2D1LinearGradientBrush *)NULL, D2D1::ColorF(start), end, gt, extendMode) {}
GradientObject::GradientObject(D2D1_COLOR_F &start, D2D1::ColorF::Enum end, AlignmentType gt, D2D1_EXTEND_MODE extendMode)
	: GradientObject((ID2D1LinearGradientBrush *)NULL, start, D2D1::ColorF(end), gt, extendMode) {}
GradientObject::GradientObject(D2D1::ColorF::Enum start, D2D1::ColorF::Enum end, AlignmentType gt, D2D1_EXTEND_MODE extendMode)
	: GradientObject((ID2D1LinearGradientBrush *)NULL, D2D1::ColorF(start), D2D1::ColorF(end), gt, extendMode) {}

GradientObject::GradientObject(ID2D1LinearGradientBrush *b, D2D1_COLOR_F &start, D2D1_COLOR_F &end, AlignmentType gt, D2D1_EXTEND_MODE extendMode)
	: GenericBrush(b), start(start), end(end), gradientType(gt), extendMode(extendMode) {}

void PaintbrushObject::Create()
{
	ID2D1SolidColorBrush *b;
	renderer->Screen->CreateSolidColorBrush(colour, &b);
	resource = boost::intrusive_ptr<ID2D1Resource>(b, false);
}

void GradientObject::Create()
{
	HRESULT hr;
	ID2D1GradientStopCollection *pGradientStops = NULL;
	ID2D1LinearGradientBrush *b;

	D2D1_GRADIENT_STOP gradientStops[] = {
		{ 0.0f, start }, { 1.0f, end }
	};

	hr = renderer->Screen->CreateGradientStopCollection(gradientStops, 2, D2D1_GAMMA_2_2, extendMode, &pGradientStops);

	if (SUCCEEDED(hr))
		hr = renderer->Screen->CreateLinearGradientBrush(
		D2D1::LinearGradientBrushProperties(D2D1::Point2F(0, 0), D2D1::Point2F(1, 0)),
		pGradientStops, &b);

	if (SUCCEEDED(hr))
	{
		SafeRelease(&pGradientStops);
		resource = boost::intrusive_ptr<ID2D1Resource>(b, false);
	}
}

void ImageBrushObject::Create()
{
	ID2D1BitmapBrush *b;
	renderer->Screen->CreateBitmapBrush(bitmap->GetImage(), properties, &b);
	resource = boost::intrusive_ptr<ID2D1Resource>(b, false);
}

void ImageObject::Create()
{
	HRESULT hr;
	ID2D1Bitmap1 *pBitmap;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void *pImageFile = NULL;
	DWORD imageFileSize = 0;

	// Create blank bitmap
	if (w != -1 && h != -1)
	{
		D2D1_BITMAP_PROPERTIES1 prop = D2D1::BitmapProperties1(bitmapOptions, pixelFormat);
		hr = renderer->Screen->CreateBitmap(D2D1::SizeU(w, h), nullptr, w * 4, &prop, &pBitmap);
	}

	// Load from resource
	else if (resourceName || resourceNameInt != -1)
	{
		if (resourceName && resourceType)
			imageResHandle = FindResource(HINST_THISCOMPONENT, resourceName, resourceType);

		else if (resourceName && resourceTypeInt != -1)
			imageResHandle = FindResource(HINST_THISCOMPONENT, resourceName, MAKEINTRESOURCE(resourceTypeInt));

		else if (resourceNameInt != -1 && resourceType)
			imageResHandle = FindResource(HINST_THISCOMPONENT, MAKEINTRESOURCE(resourceNameInt), resourceType);

		else if (resourceNameInt != -1 && resourceTypeInt != -1)
			imageResHandle = FindResource(HINST_THISCOMPONENT, MAKEINTRESOURCE(resourceNameInt), MAKEINTRESOURCE(resourceTypeInt));

		hr = imageResHandle ? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			// Load the resource
			imageResDataHandle = LoadResource(HINST_THISCOMPONENT, imageResHandle);

			hr = imageResDataHandle ? S_OK : E_FAIL;
		}

		if (SUCCEEDED(hr))
		{
			// Lock it to get a system memory pointer
			pImageFile = LockResource(imageResDataHandle);

			hr = pImageFile ? S_OK : E_FAIL;
		}

		if (SUCCEEDED(hr))
		{
			// Calculate the image file size
			imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);

			hr = imageFileSize ? S_OK : E_FAIL;
		}

		if (SUCCEEDED(hr))
		{
			// Create a WIC stream to map onto the memory
			hr = renderer->ImageFactory->CreateStream(&pStream);
		}

		if (SUCCEEDED(hr))
		{
			// Initialize the stream with the memory pointer and size
			hr = pStream->InitializeFromMemory(reinterpret_cast<BYTE*>(pImageFile), imageFileSize);
		}

		if (SUCCEEDED(hr))
		{
			// Create a decoder for the stream
			hr = renderer->ImageFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnLoad, &pDecoder);
		}
	}

	// Load from file
	else if (resourceFile)
	{
		hr = renderer->ImageFactory->CreateDecoderFromFilename(resourceFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);

		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			MessageBoxS("Failed to load image resource: " + StringFactory(resourceFile));
			exit(0);
		}
	}

	// Process image if not a blank bitmap
	if (w == -1 && h == -1)
	{
		if (SUCCEEDED(hr))
		{
			// Create the initial frame
			hr = pDecoder->GetFrame(0, &pSource);
		}

		if (SUCCEEDED(hr))
		{
			// Convert the image format to 32bppPBGRA
			// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED)
			hr = renderer->ImageFactory->CreateFormatConverter(&pConverter);
		}

		if (SUCCEEDED(hr))
		{
			hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		}

		if (SUCCEEDED(hr))
		{
			// Create Direct2D bitmap
			hr = renderer->Screen->CreateBitmapFromWicBitmap(pConverter, NULL, &pBitmap);
		}
	}

	// Free interfaces
	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	resource = boost::intrusive_ptr<ID2D1Resource>(pBitmap, false);
}

Paintbrush Simple2D::MakeBrush(D2D1_COLOR_F &col)
{
	Paintbrush p = addRenderingObject(new PaintbrushObject(col), true);

	// When we use eg. MakeBrush(Colour::White) we may get back an already-used brush with the opacity changed;
	// the user will not be expecting this to change it back to 1.0 if necessary
	if (p->GetIfCreated())
		p->SetOpacity(1.0f);

	return p;
}

Paintbrush Simple2D::MakeBrush(D2D1::ColorF::Enum col)
{
	return MakeBrush(D2D1::ColorF(col));
}

Gradient Simple2D::MakeBrush(D2D1_COLOR_F &start, D2D1_COLOR_F &end, AlignmentType gt, D2D1_EXTEND_MODE extendMode, bool managed)
{
	return addRenderingObject(new GradientObject(start, end, gt, extendMode), managed);
}

Gradient Simple2D::MakeBrush(D2D1::ColorF::Enum start, D2D1_COLOR_F &end, AlignmentType gt, D2D1_EXTEND_MODE extendMode, bool managed)
{
	return MakeBrush(D2D1::ColorF(start), end, gt, extendMode);
}

Gradient Simple2D::MakeBrush(D2D1_COLOR_F &start, D2D1::ColorF::Enum end, AlignmentType gt, D2D1_EXTEND_MODE extendMode, bool managed)
{
	return MakeBrush(start, D2D1::ColorF(end), gt, extendMode);
}

Gradient Simple2D::MakeBrush(D2D1::ColorF::Enum start, D2D1::ColorF::Enum end, AlignmentType gt, D2D1_EXTEND_MODE extendMode, bool managed)
{
	return MakeBrush(D2D1::ColorF(start), D2D1::ColorF(end), gt, extendMode);
}

ImageBrush Simple2D::MakeBrush(Image image, AlignmentType alignment, D2D1_EXTEND_MODE extendModeX, D2D1_EXTEND_MODE extendModeY, bool managed)
{
	return addRenderingObject(new ImageBrushObject(image, alignment, extendModeX, extendModeY), managed);
}

void GradientObject::SetPoints(int x1, int y1, int x2, int y2)
{
	ID2D1LinearGradientBrush *b = reinterpret_cast<ID2D1LinearGradientBrush *>(GetBrush());
	b->SetStartPoint(D2D1::Point2F(static_cast<FLOAT>(x1), static_cast<FLOAT>(y1)));
	b->SetEndPoint(D2D1::Point2F(static_cast<FLOAT>(x2), static_cast<FLOAT>(y2)));
}

void GradientObject::SetPointsWH(int x1, int y1, int w, int h)
{
	SetPoints(x1, y1, x1 + w, y1 + h);
}

void GradientObject::SetPointsUsingAlignmentType(int x1, int y1, int x2, int y2)
{
	switch (gradientType)
	{
	case Horizontal: SetPoints(x1, 0, x2, 0); break;
	case Vertical: SetPoints(0, y1, 0, y2); break;
	case Diagonal: SetPoints(x1, y1, x2, y2); break;
	case Custom: break;
	}
}

void GradientObject::SetPointsUsingAlignmentTypeWH(int x1, int y1, int w, int h)
{
	SetPointsUsingAlignmentType(x1, y1, x1 + w, y1 + h);
}

// Bitmaps
Image Simple2D::MakeImage(char const *resourceName, char const *resourceType, bool managed)
{
	return addRenderingObject(new ImageObject(resourceName, resourceType), managed);
}

Image Simple2D::MakeImage(int const resourceName, char const *resourceType, bool managed)
{
	return addRenderingObject(new ImageObject(resourceName, resourceType), managed);
}

Image Simple2D::MakeImage(char const *resourceName, int const resourceType, bool managed)
{
	return addRenderingObject(new ImageObject(resourceName, resourceType), managed);
}

Image Simple2D::MakeImage(int const resourceName, int const resourceType, bool managed)
{
	return addRenderingObject(new ImageObject(resourceName, resourceType), managed);
}

Image Simple2D::MakeImage(PCWSTR resourceFile, bool managed)
{
	return addRenderingObject(new ImageObject(resourceFile), managed);
}

Image Simple2D::MakeEmptyImage(int w, int h, D2D1_PIXEL_FORMAT pixelFormat, D2D1_BITMAP_OPTIONS options, bool managed)
{
	return addRenderingObject(new ImageObject(w, h, pixelFormat, options), managed);
}

// Set current brush
void Simple2D::SetBrush(GenericBrush *brush)
{
	CurrentBrush = brush;
}

void Simple2D::SetBrush(TemporaryBrush brush)
{
	CurrentBrush = brush.get();
}

void Simple2D::SetBrush(D2D1_COLOR_F &col)
{
	SetBrush(MakeBrush(col));
}

void Simple2D::SetBrush(D2D1::ColorF::Enum col)
{
	SetBrush(D2D1::ColorF(col));
}

// Text functions
TextFormat Simple2D::MakeTextFormat(WCHAR *font, FLOAT size, DWRITE_TEXT_ALIGNMENT alignment,
											DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch)
{
	IDWriteTextFormat *format;
	TextFactory->CreateTextFormat(font, NULL, weight, style, stretch, size, L"", &format);
	format->SetTextAlignment(alignment);

	return TextFormat(format, false);
}

TextLayout Simple2D::MakeTextLayout(const WCHAR *text, TextFormat format, int boundX, int boundY)
{
	IDWriteTextLayout *layout;
	TextFactory->CreateTextLayout(text, wcslen(text), format.get(),
		static_cast<float>((boundX == -1)? ResolutionX : boundX),
		static_cast<float>((boundY == -1)? ResolutionY : boundY), &layout);

	return TextLayout(layout, false);
}

TextLayout Simple2D::MakeTextLayout(string text, TextFormat format, int boundX, int boundY)
{	
	wchar_t *wcstring = new wchar_t[text.length() + 1];
	size_t chars;
	mbstowcs_s(&chars, wcstring, text.length() + 1, text.c_str(), _TRUNCATE);
	TextLayout layout = MakeTextLayout(wcstring, format, boundX, boundY);
	delete [] wcstring;
	return layout;
}

// Get the length of a text string in characters
int Simple2D::TextLength(TextLayout layout)
{
	UINT num;
	layout->GetClusterMetrics(NULL, 0, &num);
	return static_cast<int>(num);
}

// Get the width of a text string in pixels;
// the whole string if which is not specified, otherwise a single character
int Simple2D::TextWidth(TextLayout layout, int which)
{
	// Total width
	if (which == -1)
	{
		DWRITE_TEXT_METRICS textMetrics;
		layout->GetMetrics(&textMetrics);
		return static_cast<int>(textMetrics.widthIncludingTrailingWhitespace);
	}

	// Otherwise, the width of a single character (cluster)
	int length = TextLength(layout);
	UINT32 len2;
	DWRITE_CLUSTER_METRICS *metrics = new DWRITE_CLUSTER_METRICS[length];

	layout->GetClusterMetrics(metrics, length, &len2);

	float width = metrics[which].width;

	delete [] metrics;

	return static_cast<int>(width);
}

// Get the height of a text string in pixels
int Simple2D::TextHeight(TextLayout layout)
{
	DWRITE_TEXT_METRICS metrics;
	layout->GetMetrics(&metrics);
	return static_cast<int>(metrics.height);
}

void Simple2D::SetTextFormat(TextFormat format)
{
	CurrentTextFormat = format;
}

// Drawing functions

// Lines
void Simple2D::Line(int x1, int y1, int x2, int y2, ID2D1Brush *brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Screen->DrawLine(D2D1::Point2F(static_cast<FLOAT>(x1), static_cast<FLOAT>(y1)), D2D1::Point2F(static_cast<FLOAT>(x2), static_cast<FLOAT>(y2)), brush, strokeWidth, strokeStyle);
}

void Simple2D::Line(int x1, int y1, int x2, int y2, GenericBrush *brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	if (brush == NULL)
		brush = CurrentBrush;

	brush->Prepare(x1, y1, x2, y2);
	Line(x1, y1, x2, y2, brush->GetBrush(), strokeWidth, strokeStyle);
}

void Simple2D::Line(int x1, int y1, int x2, int y2, TemporaryBrush brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x2, y2, brush.get(), strokeWidth, strokeStyle);
}

void Simple2D::Line(int x1, int y1, int x2, int y2, D2D1_COLOR_F &col, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x2, y2, MakeBrush(col), strokeWidth, strokeStyle);
}

void Simple2D::Line(int x1, int y1, int x2, int y2, D2D1::ColorF::Enum col, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x2, y2, MakeBrush(col), strokeWidth, strokeStyle);
}

void Simple2D::Line(int x1, int y1, int x2, int y2, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x2, y2, CurrentBrush, strokeWidth, strokeStyle);
}

void Simple2D::LineWH(int x1, int y1, int w, int h, ID2D1Brush *brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x1 + w, y1 + h, brush, strokeWidth, strokeStyle);
}

void Simple2D::LineWH(int x1, int y1, int w, int h, GenericBrush *brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x1 + w, y1 + h, brush, strokeWidth, strokeStyle);
}

void Simple2D::LineWH(int x1, int y1, int w, int h, TemporaryBrush brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x1 + w, y1 + h, brush, strokeWidth, strokeStyle);
}

void Simple2D::LineWH(int x1, int y1, int w, int h, D2D1_COLOR_F &col, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x1 + w, y1 + h, MakeBrush(col), strokeWidth, strokeStyle);
}

void Simple2D::LineWH(int x1, int y1, int w, int h, D2D1::ColorF::Enum col, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x1 + w, y1 + h, MakeBrush(col), strokeWidth, strokeStyle);
}

void Simple2D::LineWH(int x1, int y1, int w, int h, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Line(x1, y1, x1 + w, y1 + h, strokeWidth, strokeStyle);
}

// Rectangles
void Simple2D::DrawRectangle(int x1, int y1, int x2, int y2, ID2D1Brush *brush)
{
	D2D1_RECT_F rect = D2D1::RectF(static_cast<FLOAT>(x1), static_cast<FLOAT>(y1), static_cast<FLOAT>(x2), static_cast<FLOAT>(y2));

	Screen->DrawRectangle(&rect, brush);
}

void Simple2D::DrawRectangle(int x1, int y1, int x2, int y2, GenericBrush *brush)
{
	if (brush == NULL)
		brush = CurrentBrush;

	brush->Prepare(x1, y1, x2, y2);
	DrawRectangle(x1, y1, x2, y2, brush->GetBrush());
}

void Simple2D::DrawRectangle(int x1, int y1, int x2, int y2, TemporaryBrush brush)
{
	DrawRectangle(x1, y1, x2, y2, brush.get());
}

void Simple2D::DrawRectangle(int x1, int y1, int x2, int y2, D2D1_COLOR_F &col)
{
	DrawRectangle(x1, y1, x2, y2, MakeBrush(col));
}

void Simple2D::DrawRectangle(int x1, int y1, int x2, int y2, D2D1::ColorF::Enum col)
{
	DrawRectangle(x1, y1, x2, y2, MakeBrush(col));
}

void Simple2D::DrawRectangleWH(int x1, int y1, int w, int h, ID2D1Brush *brush)
{
	DrawRectangle(x1, y1, x1 + w, y1 + h, brush);
}

void Simple2D::DrawRectangleWH(int x1, int y1, int w, int h, GenericBrush *brush)
{
	DrawRectangle(x1, y1, x1 + w, y1 + h, brush);
}

void Simple2D::DrawRectangleWH(int x1, int y1, int w, int h, TemporaryBrush brush)
{
	DrawRectangle(x1, y1, x1 + w, y1 + h, brush);
}

void Simple2D::DrawRectangleWH(int x1, int y1, int w, int h, D2D1_COLOR_F &col)
{
	DrawRectangleWH(x1, y1, w, h, MakeBrush(col));
}

void Simple2D::DrawRectangleWH(int x1, int y1, int w, int h, D2D1::ColorF::Enum col)
{
	DrawRectangleWH(x1, y1, w, h, MakeBrush(col));
}

void Simple2D::FillRectangle(int x1, int y1, int x2, int y2, ID2D1Brush *brush)
{
	D2D1_RECT_F rect = D2D1::RectF(static_cast<FLOAT>(x1), static_cast<FLOAT>(y1), static_cast<FLOAT>(x2), static_cast<FLOAT>(y2));

	Screen->FillRectangle(&rect, brush);
}

void Simple2D::FillRectangle(int x1, int y1, int x2, int y2, GenericBrush *brush)
{
	if (brush == NULL)
		brush = CurrentBrush;

	brush->Prepare(x1, y1, x2, y2);
	FillRectangle(x1, y1, x2, y2, brush->GetBrush());
}

void Simple2D::FillRectangle(int x1, int y1, int x2, int y2, TemporaryBrush brush)
{
	FillRectangle(x1, y1, x2, y2, brush.get());
}

void Simple2D::FillRectangle(int x1, int y1, int x2, int y2, D2D1_COLOR_F &col)
{
	FillRectangle(x1, y1, x2, y2, MakeBrush(col));
}

void Simple2D::FillRectangle(int x1, int y1, int x2, int y2, D2D1::ColorF::Enum col)
{
	FillRectangle(x1, y1, x2, y2, MakeBrush(col));
}

void Simple2D::FillRectangleWH(int x1, int y1, int w, int h, ID2D1Brush *brush)
{
	FillRectangle(x1, y1, x1 + w, y1 + h, brush);
}

void Simple2D::FillRectangleWH(int x1, int y1, int w, int h, GenericBrush *brush)
{
	FillRectangle(x1, y1, x1 + w, y1 + h, brush);
}

void Simple2D::FillRectangleWH(int x1, int y1, int w, int h, TemporaryBrush brush)
{
	FillRectangle(x1, y1, x1 + w, y1 + h, brush);
}

void Simple2D::FillRectangleWH(int x1, int y1, int w, int h, D2D1_COLOR_F &col)
{
	FillRectangleWH(x1, y1, w, h, MakeBrush(col));
}

void Simple2D::FillRectangleWH(int x1, int y1, int w, int h, D2D1::ColorF::Enum col)
{
	FillRectangleWH(x1, y1, w, h, MakeBrush(col));
}

// Rounded rectangles
void Simple2D::DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, ID2D1Brush *brush)
{
	D2D1_ROUNDED_RECT rect = D2D1::RoundedRect(
		D2D1::RectF(static_cast<FLOAT>(x1), static_cast<FLOAT>(y1), static_cast<FLOAT>(x2), static_cast<FLOAT>(y2)),
					static_cast<FLOAT>(hr), static_cast<float>(vr));

	Screen->DrawRoundedRectangle(&rect, brush);
}

void Simple2D::DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, GenericBrush *brush)
{
	if (brush == NULL)
		brush = CurrentBrush;

	brush->Prepare(x1, y1, x2, y2);
	DrawRoundedRectangle(x1, y1, x2, y2, hr, vr, brush->GetBrush());
}
	
void Simple2D::DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, TemporaryBrush brush)
{
	DrawRoundedRectangle(x1, y1, x2, y2, hr, vr, brush.get());
}
	
void Simple2D::DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, D2D1_COLOR_F &col)
{
	DrawRoundedRectangle(x1, y1, x2, y2, hr, vr, MakeBrush(col));
}

void Simple2D::DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, D2D1::ColorF::Enum col)
{
	DrawRoundedRectangle(x1, y1, x2, y2, hr, vr, MakeBrush(col));
}

void Simple2D::DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, ID2D1Brush *brush)
{
	DrawRoundedRectangle(x1, y1, x1 + w, y1 + h, hr, vr, brush);
}

void Simple2D::DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, GenericBrush *brush)
{
	DrawRoundedRectangle(x1, y1, x1 + w, y1 + h, hr, vr, brush);
}

void Simple2D::DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, TemporaryBrush brush)
{
	DrawRoundedRectangle(x1, y1, x1 + w, y1 + h, hr, vr, brush);
}

void Simple2D::DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, D2D1_COLOR_F &col)
{
	DrawRoundedRectangleWH(x1, y1, w, h, hr, vr, MakeBrush(col));
}

void Simple2D::DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, D2D1::ColorF::Enum col)
{
	DrawRoundedRectangleWH(x1, y1, w, h, hr, vr, MakeBrush(col));
}

void Simple2D::FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, ID2D1Brush *brush)
{
	D2D1_ROUNDED_RECT rect = D2D1::RoundedRect(
		D2D1::RectF(static_cast<FLOAT>(x1), static_cast<FLOAT>(y1), static_cast<FLOAT>(x2), static_cast<FLOAT>(y2)),
					static_cast<FLOAT>(hr), static_cast<float>(vr));

	Screen->FillRoundedRectangle(&rect, brush);
}

void Simple2D::FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, GenericBrush *brush)
{
	if (brush == NULL)
		brush = CurrentBrush;

	brush->Prepare(x1, y1, x2, y2);
	FillRoundedRectangle(x1, y1, x2, y2, hr, vr, brush->GetBrush());
}

void Simple2D::FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, TemporaryBrush brush)
{
	FillRoundedRectangle(x1, y1, x2, y2, hr, vr, brush.get());
}

void Simple2D::FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, D2D1_COLOR_F &col)
{
	FillRoundedRectangle(x1, y1, x2, y2, hr, vr, MakeBrush(col));
}

void Simple2D::FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, D2D1::ColorF::Enum col)
{
	FillRoundedRectangle(x1, y1, x2, y2, hr, vr, MakeBrush(col));
}

void Simple2D::FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, ID2D1Brush *brush)
{
	FillRoundedRectangle(x1, y1, x1 + w, y1 + h, hr, vr, brush);
}

void Simple2D::FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, GenericBrush *brush)
{
	FillRoundedRectangle(x1, y1, x1 + w, y1 + h, hr, vr, brush);
}

void Simple2D::FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, TemporaryBrush brush)
{
	FillRoundedRectangle(x1, y1, x1 + w, y1 + h, hr, vr, brush);
}

void Simple2D::FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, D2D1_COLOR_F &col)
{
	FillRoundedRectangleWH(x1, y1, w, h, hr, vr, MakeBrush(col));
}

void Simple2D::FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, D2D1::ColorF::Enum col)
{
	FillRoundedRectangleWH(x1, y1, w, h, hr, vr, MakeBrush(col));
}

// Text
void Simple2D::Text(int x, int y, string text, WCHAR *font, FLOAT size, ID2D1Brush *brush, DWRITE_TEXT_ALIGNMENT alignment,
					DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch, int w, int h)
{
	IDWriteTextFormat *format;
	TextFactory->CreateTextFormat(font, NULL, weight, style, stretch, size, L"", &format);
	format->SetTextAlignment(alignment);

	Text(x, y, text, brush, format, w, h);
	SafeRelease(&format);
}

void Simple2D::Text(int x, int y, string text, WCHAR *font, FLOAT size, GenericBrush *brush, DWRITE_TEXT_ALIGNMENT alignment,
					DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch, int w, int h)
{
	if (brush == NULL)
		brush = CurrentBrush;

	IDWriteTextFormat *format;
	TextFactory->CreateTextFormat(font, NULL, weight, style, stretch, size, L"", &format);
	format->SetTextAlignment(alignment);

	// Figure out how wide and tall the text is
	TextLayout layout = MakeTextLayout(text, format, (w == -1? ResolutionX : x + w), (h == -1? ResolutionY : y + h));
	DWRITE_TEXT_METRICS metrics;
	layout->GetMetrics(&metrics);
	brush->Prepare(x, y, x + static_cast<int>(metrics.width), y + static_cast<int>(metrics.height));

	Text(x, y, text, brush, format, w, h);
	SafeRelease(&format);
}

void Simple2D::Text(int x, int y, string text, WCHAR *font, FLOAT size, TemporaryBrush brush, DWRITE_TEXT_ALIGNMENT alignment,
					DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch, int w, int h)
{
	Text(x, y, text, font, size, brush.get(), alignment, weight, style, stretch, w, h);
}

void Simple2D::Text(int x, int y, string text, WCHAR *font, FLOAT size, D2D1::ColorF::Enum col, DWRITE_TEXT_ALIGNMENT alignment,
					DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch, int w, int h)
{
	Text(x, y, text, font, size, MakeBrush(col), alignment, weight, style, stretch, w, h);
}

void Simple2D::Text(int x, int y, string text, WCHAR *font, FLOAT size, D2D1_COLOR_F &col, DWRITE_TEXT_ALIGNMENT alignment,
					DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch, int w, int h)
{
	Text(x, y, text, font, size, MakeBrush(col), alignment, weight, style, stretch, w, h);
}

void Simple2D::Text(int x, int y, string text, ID2D1Brush *brush, TextFormat format, int w, int h)
{
	if (format == NULL)
		format = CurrentTextFormat;

	if (w == -1)
		w = ResolutionX - x;

	if (h == -1)
		h = ResolutionY - y;

	D2D1_RECT_F rect = D2D1::RectF(static_cast<FLOAT>(x), static_cast<FLOAT>(y),
								   static_cast<FLOAT>(x + w), static_cast<FLOAT>(y + h));

	wchar_t *wcstring = new wchar_t[text.length() + 1];
	size_t chars;

	mbstowcs_s(&chars, wcstring, text.length() + 1, text.c_str(), _TRUNCATE);

	Screen->DrawTextA(wcstring, text.length(), format.get(), rect, brush);

	delete [] wcstring;
}

void Simple2D::Text(int x, int y, string text, GenericBrush *brush, TextFormat format, int w, int h)
{
	if (brush == NULL)
		brush = CurrentBrush;

	if (format == NULL)
		format = CurrentTextFormat;

	// Figure out how wide and tall the text is
	TextLayout layout = MakeTextLayout(text, format, (w == -1? ResolutionX : x + w), (h == -1? ResolutionY : y + h));
	DWRITE_TEXT_METRICS metrics;
	layout->GetMetrics(&metrics);
	brush->Prepare(x, y, x + static_cast<int>(metrics.width), y + static_cast<int>(metrics.height));

	Text(x, y, text, brush->GetBrush(), format, w, h);
}

void Simple2D::Text(int x, int y, string text, TemporaryBrush brush, TextFormat format, int w, int h)
{
	Text(x, y, text, brush.get(), format, w, h);
}

void Simple2D::Text(int x, int y, string text, D2D1_COLOR_F &col, TextFormat format, int w, int h)
{
	Text(x, y, text, MakeBrush(col), format, w, h);
}

void Simple2D::Text(int x, int y, string text, D2D1::ColorF::Enum col, TextFormat format, int w, int h)
{
	Text(x, y, text, MakeBrush(col), format, w, h);
}

// Path geometry
Geometry Simple2D::MakeEllipseGeometry(int x, int y, int rx, int ry)
{
	ID2D1EllipseGeometry *geom;

	// Circles
	if (ry == - 1) ry = rx;

	HRESULT hr = Direct2D->CreateEllipseGeometry(D2D1::Ellipse(D2D1::Point2F(
		static_cast<float>(x), static_cast<float>(y)), static_cast<float>(rx), static_cast<float>(ry)), &geom);

	if (SUCCEEDED(hr))
		return Geometry(geom, Geometry::Filled, Default);

	return Geometry();
}

Geometry Simple2D::MakeEllipseGeometry(int rx, int ry)
{
	return MakeEllipseGeometry(0, 0, rx, ry);
}

Geometry Simple2D::MakeRectangleGeometry(int x1, int y1, int x2, int y2)
{
	ID2D1RectangleGeometry *geom;

	HRESULT hr = Direct2D->CreateRectangleGeometry(D2D1::RectF(
		static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(x2), static_cast<float>(y2)), &geom);

	if (SUCCEEDED(hr))
		return Geometry(geom, Geometry::Filled, Default);
	
	return Geometry();
}

Geometry Simple2D::MakeRectangleGeometry(D2D1_RECT_F r)
{
	return MakeRectangleGeometry(static_cast<int>(r.left), static_cast<int>(r.top), static_cast<int>(r.right), static_cast<int>(r.bottom));
}

Geometry Simple2D::MakeRectangleGeometryWH(int x1, int y1, int w, int h)
{
	return MakeRectangleGeometry(x1, y1, x1 + w, y1 + h);
}

Geometry Simple2D::MakeRectangleGeometry(int w, int h)
{
	return MakeRectangleGeometry(0, 0, w, h);
}

ID2D1GeometrySink *Simple2D::StartCreatePath(int x, int y, Geometry::FillType ft, GeometryDrawStart rp, Geometry::FigureFillType fft)
{
	ID2D1PathGeometry *geom;

	HRESULT hr = Direct2D->CreatePathGeometry(&geom);

	if (SUCCEEDED(hr))
	{
		pathToCreate = Geometry(geom, ft, rp);

		ID2D1GeometrySink *sink = pathToCreate.OpenSink();

		if (fft == Geometry::Winding)
			sink->SetFillMode(D2D1_FILL_MODE_WINDING); 
		if (fft == Geometry::Alternate)
			sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);

		sink->BeginFigure(D2D1::Point2F(static_cast<FLOAT>(x), static_cast<FLOAT>(y)), (ft == Geometry::Filled? D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW));

		return sink;
	}
	return (ID2D1GeometrySink *)0;
}

Geometry Simple2D::EndCreatePath(Geometry::PathType pt)
{
	ID2D1GeometrySink *sink = pathToCreate.OpenSink();
	sink->EndFigure((pt == Geometry::Open? D2D1_FIGURE_END_OPEN : D2D1_FIGURE_END_CLOSED));

	pathToCreate.CloseSink();
	return pathToCreate;
}

Geometry Simple2D::MakeLineGeometry(D2D1_POINT_2F *points, int numPoints, Geometry::PathType pt, Geometry::FillType ft, GeometryDrawStart rp, Geometry::FigureFillType fft)
{
	GeometryData gd = StartCreatePath(static_cast<int>(points[0].x), static_cast<int>(points[0].y), ft, rp, fft);
	gd->AddLines(points + 1, numPoints - 1);
	return EndCreatePath(pt);
}

Geometry Geometry::GetGeometry(Matrix &m)
{
	ID2D1Geometry *g = static_cast<ID2D1Geometry *>(Get());
	ID2D1TransformedGeometry *t = internalTransform(m);
	return Geometry(t, fillType, relativeDrawPos);
}

ID2D1GeometrySink *Geometry::OpenSink()
{
	if (sink == NULL)
		static_cast<ID2D1PathGeometry *>(resource.get())->Open(&sink);

	return sink;
}

void Geometry::CloseSink()
{
	sink->Close();
	SafeRelease(&sink);
}

D2D1_RECT_F Geometry::GetOriginalBounds(Matrix &trans, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	ID2D1Geometry &geom = *static_cast<ID2D1Geometry *>(resource.get());

	D2D1_MATRIX_3X2_F wt;
	renderer->Screen->GetTransform(&wt);

	D2D1_RECT_F bounds;
	if (fillType == Filled)
		geom.GetBounds(trans * wt, &bounds);
	else
		geom.GetWidenedBounds(strokeWidth < 0.f? defaultStrokeWidth : strokeWidth, strokeStyle == NULL? defaultStrokeStyle : strokeStyle, trans * wt, &bounds);

	return bounds;
}

D2D1_RECT_F Geometry::GetBounds(Matrix &trans, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	ID2D1Geometry &geom = *static_cast<ID2D1Geometry *>(resource.get());

	D2D1_MATRIX_3X2_F wt;
	renderer->Screen->GetTransform(&wt);

	D2D1_RECT_F bounds;
	if (fillType == Filled)
		geom.GetBounds(transform * trans * wt, &bounds);
	else
		geom.GetWidenedBounds(strokeWidth < 0.f? defaultStrokeWidth : strokeWidth, strokeStyle == NULL? defaultStrokeStyle : strokeStyle, transform * trans * wt, &bounds);

	return bounds;
}

float Geometry::GetOriginalLength(Matrix &trans)
{
	return GetOriginalLength(&trans);
}

float Geometry::GetOriginalLength(Matrix *trans)
{
	float length;
	static_cast<ID2D1Geometry *>(resource.get())->ComputeLength(trans, &length);
	return length;
}

float Geometry::GetLength(Matrix &trans)
{
	return GetLength(&trans);
}

float Geometry::GetLength(Matrix *trans)
{
	float length;

	if (trans == NULL)
		static_cast<ID2D1Geometry *>(resource.get())->ComputeLength(transform, &length);
	else
		static_cast<ID2D1Geometry *>(resource.get())->ComputeLength(transform * *trans, &length);

	return length;
}

// NOTE: It is the responsibility of the application to free this geometry!
// Should generally be for internal use only
ID2D1TransformedGeometry *Geometry::internalTransform(Matrix &trans, GeometryDrawStart relPos, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	ID2D1TransformedGeometry *t;
	D2D1_MATRIX_3X2_F fullTransform;

	if (strokeWidth < 0.f)
		strokeWidth = defaultStrokeWidth;

	if (strokeStyle == NULL)
		strokeStyle = defaultStrokeStyle;

	D2D1_RECT_F bounds = GetOriginalBounds(D2D1::Matrix3x2F::Identity(), strokeWidth, strokeStyle);

	if (relPos == Assigned)
		relPos = relativeDrawPos;
	
	if (relPos == Default)
		fullTransform = transform * trans;

	else if (relPos == TopLeft)
		fullTransform = transform * trans * D2D1::Matrix3x2F::Translation(-bounds.left, -bounds.top);

	else if (relPos == Center)
		fullTransform = transform * trans * D2D1::Matrix3x2F::Translation((bounds.left - bounds.right) / 2 - bounds.left, (bounds.top - bounds.bottom) / 2 - bounds.top);

	renderer->Direct2D->CreateTransformedGeometry(GetOriginalGeometry(), fullTransform, &t);

	return t;
}

D2D1_GEOMETRY_RELATION Geometry::GetIntersection(Geometry &o)
{
	D2D1_GEOMETRY_RELATION rel;

	GetGeometry().GetOriginalGeometry()->CompareWithGeometry(o.GetGeometry().GetOriginalGeometry(), D2D1::Matrix3x2F::Identity(), &rel);

	return rel;
}

D2D1_GEOMETRY_RELATION Simple2D::GeometryCollision(Geometry &o1, Geometry &o2, Matrix &m1, Matrix &m2)
{
	D2D1_GEOMETRY_RELATION rel;

	Geometry t = o1.GetGeometry(m1);
	t.GetOriginalGeometry()->CompareWithGeometry(o2.GetGeometry().GetOriginalGeometry(), m2, &rel);

	return rel;
}

Geometry Geometry::GetIntersectedGeometry(Geometry &o, Matrix &m)
{
	ID2D1PathGeometry *geom;

	HRESULT hr = renderer->Direct2D->CreatePathGeometry(&geom);

	Geometry pathToCreate = Geometry(geom);

	ID2D1GeometrySink *sink = pathToCreate.OpenSink();

	GetGeometry().GetOriginalGeometry()->CombineWithGeometry(o.GetGeometry().GetOriginalGeometry(),
															D2D1_COMBINE_MODE_INTERSECT, m, sink);

	pathToCreate.CloseSink();
	return pathToCreate;
}

bool Geometry::ContainsPoint(D2D1_POINT_2F point)
{
	D2D1_MATRIX_3X2_F wt;
	renderer->Screen->GetTransform(&wt);

	BOOL contains;
	GetGeometry().GetOriginalGeometry()->FillContainsPoint(point, wt, &contains);
	return (contains == TRUE);
}

bool Geometry::ContainsPoint(float x, float y)
{
	return ContainsPoint(D2D1::Point2F(x, y));
}

bool Geometry::ContainsPoint(int x, int y)
{
	return ContainsPoint(D2D1::Point2F(static_cast<FLOAT>(x), static_cast<FLOAT>(y)));
}

Matrix Geometry::Rotate(float angle, GeometryTransformPoint point)
{
	D2D1_RECT_F bounds = GetBounds();
	D2D1_POINT_2F tp;

	switch (point) {

	case PointTopLeft:
		tp.x = bounds.left; tp.y = bounds.top;
		break;

	case PointTopRight:
		tp.x = bounds.right; tp.y = bounds.top;
		break;

	case PointBottomLeft:
		tp.x = bounds.left; tp.y = bounds.bottom;
		break;

	case PointBottomRight:
		tp.x = bounds.right; tp.y = bounds.bottom;
		break;

	case PointCenter:
		tp.x = (bounds.right - bounds.left) / 2 + bounds.left;
		tp.y = (bounds.bottom - bounds.top) / 2 + bounds.top;
		break;

	}
	return D2D1::Matrix3x2F::Rotation(angle, tp);
}

Matrix Geometry::Rotate(float angle, int x, int y)
{
	D2D1_RECT_F bounds = GetBounds();
	D2D1_POINT_2F tp = { bounds.left + x, bounds.top + y };
	return D2D1::Matrix3x2F::Rotation(angle, tp);
}

Matrix Geometry::Scale(float scaleX, float scaleY, GeometryTransformPoint point)
{
	D2D1_RECT_F bounds = GetBounds();
	D2D1_POINT_2F tp;

	switch (point) {

	case PointTopLeft:
		tp.x = bounds.left; tp.y = bounds.top;
		break;

	case PointTopRight:
		tp.x = bounds.right; tp.y = bounds.top;
		break;

	case PointBottomLeft:
		tp.x = bounds.left; tp.y = bounds.bottom;
		break;

	case PointBottomRight:
		tp.x = bounds.right; tp.y = bounds.bottom;
		break;

	case PointCenter:
		tp.x = (bounds.right - bounds.left) / 2 + bounds.left;
		tp.y = (bounds.bottom - bounds.top) / 2 + bounds.top;
		break;

	}
	return D2D1::Matrix3x2F::Scale(scaleX, scaleY, tp);
}

Matrix Geometry::Scale(float scaleX, float scaleY, int x, int y)
{
	D2D1_RECT_F bounds = GetBounds();
	D2D1_POINT_2F tp = { bounds.left + x, bounds.top + y };
	return D2D1::Matrix3x2F::Scale(scaleX, scaleY, tp);
}

Matrix Geometry::Skew(float angleX, float angleY, GeometryTransformPoint point)
{
	D2D1_RECT_F bounds = GetBounds();
	D2D1_POINT_2F tp;

	switch (point) {

	case PointTopLeft:
		tp.x = bounds.left; tp.y = bounds.top;
		break;

	case PointTopRight:
		tp.x = bounds.right; tp.y = bounds.top;
		break;

	case PointBottomLeft:
		tp.x = bounds.left; tp.y = bounds.bottom;
		break;

	case PointBottomRight:
		tp.x = bounds.right; tp.y = bounds.bottom;
		break;

	case PointCenter:
		tp.x = (bounds.right - bounds.left) / 2 + bounds.left;
		tp.y = (bounds.bottom - bounds.top) / 2 + bounds.top;
		break;

	}
	return D2D1::Matrix3x2F::Skew(angleX, angleY, tp);
}

Matrix Geometry::Skew(float angleX, float angleY, int x, int y)
{
	D2D1_RECT_F bounds = GetBounds();
	D2D1_POINT_2F tp = { bounds.left + x, bounds.top + y };
	return D2D1::Matrix3x2F::Skew(angleX, angleY, tp);
}

Matrix Geometry::Move(float x,float y)
{
	return D2D1::Matrix3x2F::Translation(x, y);
}

Matrix Geometry::Move(int x, int y)
{
	return D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(x), static_cast<FLOAT>(y));
}

Matrix Geometry::None()
{
	return D2D1::Matrix3x2F::Identity();
}

void Geometry::Draw(D2D1_MATRIX_3X2_F &trans, GeometryDrawStart relPos, ID2D1Brush *brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	if (relPos == Assigned)
		relPos = relativeDrawPos;

	if (strokeWidth < 0.f)
		strokeWidth = defaultStrokeWidth;

	if (strokeStyle == NULL)
		strokeStyle = defaultStrokeStyle;

	ID2D1TransformedGeometry *t = internalTransform(trans, relPos, strokeWidth, strokeStyle);
	renderer->Screen->DrawGeometry(t, brush, strokeWidth, strokeStyle);
	SafeRelease(&t);
}

void Geometry::Draw(D2D1_MATRIX_3X2_F &trans, GeometryDrawStart relPos, GenericBrush *brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	if (brush == NULL)
		brush = renderer->CurrentBrush;

	if (relPos == Assigned)
		relPos = relativeDrawPos;

	if (strokeWidth < 0.f)
		strokeWidth = defaultStrokeWidth;

	if (strokeStyle == NULL)
		strokeStyle = defaultStrokeStyle;

	ID2D1TransformedGeometry *t = internalTransform(trans, relPos, strokeWidth, strokeStyle);

	// We have to get the bounds of the transformed geometry to make the gradient brush fill it correctly
	D2D1_MATRIX_3X2_F wt;
	renderer->Screen->GetTransform(&wt);

	D2D1_RECT_F bounds;
	if (fillType == Filled)
		t->GetBounds(wt, &bounds);
	else
		t->GetWidenedBounds(strokeWidth, strokeStyle, wt, &bounds);

	brush->Prepare(static_cast<int>(bounds.left), static_cast<int>(bounds.top), static_cast<int>(bounds.right), static_cast<int>(bounds.bottom));

	if (autoAdjustBrush)
		brush->SetTransform(transform * trans);

	renderer->Screen->DrawGeometry(t, brush->GetBrush(), strokeWidth, strokeStyle);
	SafeRelease(&t);
}

void Geometry::Draw(D2D1_MATRIX_3X2_F &transform, GeometryDrawStart relPos, TemporaryBrush brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Draw(transform, relPos, brush.get(), strokeWidth, strokeStyle);
}

void Geometry::Draw(int x, int y, GeometryDrawStart relPos, ID2D1Brush *brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Draw(D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(x), static_cast<FLOAT>(y)), relPos, brush, strokeWidth, strokeStyle);
}

void Geometry::Draw(int x, int y, GeometryDrawStart relPos, GenericBrush *brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Draw(D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(x), static_cast<FLOAT>(y)), relPos, brush, strokeWidth, strokeStyle);
}

void Geometry::Draw(int x, int y, GeometryDrawStart relPos, TemporaryBrush brush, FLOAT strokeWidth, ID2D1StrokeStyle *strokeStyle)
{
	Draw(D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(x), static_cast<FLOAT>(y)), relPos, brush.get(), strokeWidth, strokeStyle);
}

void Geometry::Fill(D2D1_MATRIX_3X2_F &trans, GeometryDrawStart relPos, ID2D1Brush *brush, ID2D1Brush *opacityBrush)
{
	if (relPos == Assigned)
		relPos = relativeDrawPos;

	if (opacityBrush == NULL)
		opacityBrush = defaultOpacityBrush;

	ID2D1TransformedGeometry *t = internalTransform(trans, relPos);
	renderer->Screen->FillGeometry(t, brush, opacityBrush);
	SafeRelease(&t);
}

void Geometry::Fill(D2D1_MATRIX_3X2_F &trans, GeometryDrawStart relPos, GenericBrush *brush, ID2D1Brush *opacityBrush)
{
	if (brush == NULL)
		brush = renderer->CurrentBrush;

	if (relPos == Assigned)
		relPos = relativeDrawPos;

	if (opacityBrush == NULL)
		opacityBrush = defaultOpacityBrush;

	ID2D1TransformedGeometry *t = internalTransform(trans, relPos);

	// We have to get the bounds of the transformed geometry to make the gradient brush fill it correctly
	D2D1_MATRIX_3X2_F wt;
	renderer->Screen->GetTransform(&wt);

	D2D1_RECT_F bounds;
	if (fillType == Filled)
		t->GetBounds(wt, &bounds);
	else
		t->GetWidenedBounds(1.0f, NULL, wt, &bounds);

	brush->Prepare(static_cast<int>(bounds.left), static_cast<int>(bounds.top), static_cast<int>(bounds.right), static_cast<int>(bounds.bottom));

	if (autoAdjustBrush)
		brush->SetTransform(transform * trans);

	renderer->Screen->FillGeometry(t, brush->GetBrush(), opacityBrush);
	SafeRelease(&t);
}

void Geometry::Fill(D2D1_MATRIX_3X2_F &transform, GeometryDrawStart relPos, TemporaryBrush brush, ID2D1Brush *opacityBrush)
{
	Fill(transform, relPos, brush.get(), opacityBrush);
}

void Geometry::Fill(int x, int y, GeometryDrawStart relPos, ID2D1Brush *brush, ID2D1Brush *opacityBrush)
{
	Fill(D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(x), static_cast<FLOAT>(y)), relPos, brush, opacityBrush);
}

void Geometry::Fill(int x, int y, GeometryDrawStart relPos, GenericBrush *brush, ID2D1Brush *opacityBrush)
{
	Fill(D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(x), static_cast<FLOAT>(y)), relPos, brush, opacityBrush);
}

void Geometry::Fill(int x, int y, GeometryDrawStart relPos, TemporaryBrush brush, ID2D1Brush *opacityBrush)
{
	Fill(D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(x), static_cast<FLOAT>(y)), relPos, brush.get(), opacityBrush);
}

// Bitmaps
void ImageObject::Draw(int x, int y, float opacity, float rotation)
{
	D2D1_SIZE_F size = GetImage()->GetSize();

	Draw(x, y, x + static_cast<int>(size.width), y + static_cast<int>(size.height), opacity, rotation);
}

void ImageObject::Draw(int x1, int y1, int x2, int y2, float opacity, float rotation)
{
	D2D1_SIZE_F size = GetImage()->GetSize();

	DrawPart(x1, y1, x2, y2, 0, 0, static_cast<int>(size.width), static_cast<int>(size.height), opacity, rotation);
}

void ImageObject::DrawWH(int x, int y, int w, int h, float opacity, float rotation)
{
	Draw(x, y, x + w, y + h, opacity, rotation);
}

void ImageObject::DrawPartWH(int x, int y, int srcX, int srcY, int w, int h, float opacity, float rotation)
{
	DrawPart(x, y, x + w, y + h, srcX, srcY, srcX + w, srcY + h, opacity, rotation);
}

void ImageObject::DrawPartWH(int x, int y, int w, int h, int srcX, int srcY, int srcW, int srcH, float opacity, float rotation)
{
	DrawPart(x, y, x + w, y + h, srcX, srcY, srcX + srcW, srcY + srcH, opacity, rotation);
}

void ImageObject::DrawPart(int x1, int y1, int x2, int y2, int srcX1, int srcY1, int srcX2, int srcY2, float opacity, float rotation)
{
	// Retain original render target transform
	D2D1_MATRIX_3X2_F t;
	renderer->Screen->GetTransform(&t);

	// Calculate image center on the render target
	D2D1_POINT_2F imageCenter = D2D1::Point2F(static_cast<FLOAT>(x2/2 + x1/2), static_cast<FLOAT>(y2/2 + y1/2));

	// Set transform
	renderer->Screen->SetTransform(D2D1::Matrix3x2F::Rotation(rotation, imageCenter) * t);

	// Draw bitmap
	renderer->Screen->DrawBitmap(GetImage(),
		D2D1::RectF(
		static_cast<FLOAT>(x1), static_cast<FLOAT>(y1),
		static_cast<FLOAT>(x2), static_cast<FLOAT>(y2)),
		
		opacity,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		
		D2D1::RectF(
		static_cast<FLOAT>(srcX1), static_cast<FLOAT>(srcY1),
		static_cast<FLOAT>(srcX2), static_cast<FLOAT>(srcY2)
		));

	// Restore original transform
	renderer->Screen->SetTransform(t);
}

// Get random number
int Simple2D::Random(int min, int max)
{
	std::uniform_int<> gen(min, max);
	return gen(Simple2D::RandomEngine);
}

float Simple2D::Random(float min, float max)
{
	std::uniform_real<float> gen(min, max);
	return gen(Simple2D::RandomEngine);
}

// Modified from Graphimation (MSDN Blog)
void Simple2D::ShowFPS(int fps)
{
    std::ostringstream ss;
    ss << fps;
 
	char *wn = new char[(wcslen(WindowName)+1) * 2];
	size_t converted = 0;
	wcstombs_s(&converted, wn, (wcslen(WindowName)+1) * 2, WindowName, _TRUNCATE);

	string s = string(wn) + " | fps = " + ss.str();
    SetWindowText(m_hwnd_app, s.c_str());

	delete [] wn;
}

// Set window title
void Simple2D::SetWindowTitle(LPCSTR title)
{
	SetWindowText(m_hwnd_app, title);
}

// End Simple2D namespace
}