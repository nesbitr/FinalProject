package com.Asteroids.game;

import com.badlogic.gdx.ApplicationAdapter;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;

import java.applet.*;
import java.awt.*;
import java.awt.event.*;
public class AsteroidsGame extends Applet implements Runnable, KeyListener{

 Thread thread;
 Dimension dim;
 Image img;
 Graphics g;

long endTime, startTime, framePeriod;

 Ship ship;
boolean paused; // True if the game is paused. Enter is the pause key
 Shot[] shots;
int numShots;
boolean shooting;

 Asteroid[] asteroids; //the array of asteroids
int numAsteroids; //the number of asteroids currently in the array
double astRadius, minAstVel,maxAstVel; //values used to create
//asteroids
int astNumHits,astNumSplit;

int level; //the current level number

public void init(){
 resize(500,500);

 shots=new Shot[41]; //41 is a shot's life period plus one.
 //since at most one shot can be fired per frame,
 //there will never be more than 41 shots if each one only
 //lives for 40 frames.

 numAsteroids=0;
 level=0; //will be incremented to 1 when first level is set up
 astRadius=40; //values used to create the asteroids
 minAstVel=.5;
 maxAstVel=5;
 astNumHits=3;
 astNumSplit=2;
 endTime=0;
 startTime=0;
 framePeriod=25;
 addKeyListener(this); //tell it to listen for KeyEvents
 dim=getSize();
 img=createImage(dim.width, dim.height);
 g=img.getGraphics();
 thread=new Thread(this);
 thread.start();
 }

public void setUpNextLevel(){ //start new level with one more asteroid
 level++;
 // create a new, inactive ship centered on the screen
 // I like .35 for acceleration, .98 for velocityDecay, and
 // .1 for rotationalSpeed. They give the controls a nice feel.
 if (level<=2){
	 ship=new Ship(250,250,0,.35,.98,.1,12);
 }
 if (level>=3){
	 ship=new Ship(250,250,0,.35,.98,.15,10);
 }
 if (level>=4){
	 ship=new Ship(250,250,0,.35,.98,.2,9);
 }
 if (level>=5){
	 ship=new Ship(250,250,0,.35,.98,.3,7);
 }
 if (level>=6){
	 ship=new Ship(250,250,0,.35,.98,.45,5);
 }
 numShots=0; //no shots on the screen at beginning of level
 paused=false;
 shooting=false;
 //create an array large enough to hold the biggest number
 //of asteroids possible on this level (plus one because
 //the split asteroids are created first, then the original
 //one is deleted). The level number is equal to the
 //number of asteroids at it's start.
 asteroids=new Asteroid[2*level *
 (int)Math.pow(astNumSplit,astNumHits-1)+1];
 numAsteroids=2*level;
 //create asteroids in random spots on the screen
 for(int i=0;i<numAsteroids;i++)
 asteroids[i]=new Asteroid(Math.random()*dim.width,
 Math.random()*dim.height,astRadius,minAstVel,
 maxAstVel,astNumHits,astNumSplit);
 }

public int returnLevel(){
	return level;
}
public void paint(Graphics gfx){
 
 if(level<=1){
	 g.setColor(Color.red);
 }
 if (level>=3){
	 g.setColor(Color.black);
 }
 if (level>=5){
	 g.setColor(Color.green);
	 
 }
 if (level>=7){
	 g.setColor(Color.darkGray);
 }
 g.fillRect(0,0,500,500);

 for(int i=0;i<numShots;i++) //draw all the shots on the screen
 shots[i].draw(g);

 for(int i=0;i<numAsteroids;i++)
 asteroids[i].draw(g);
 ship.draw(g); //draw the ship

 g.setColor(Color.cyan); //Display level number in top left corner
 g.drawString("Level " + level,20,20);

 gfx.drawImage(img,0,0,this);
 }

public void update(Graphics gfx){
 paint(gfx);
 }

public void run(){
 for(;;){
 startTime=System.currentTimeMillis();

 //start next level when all asteroids are destroyed
 if(numAsteroids<=0)
 setUpNextLevel();

 if(!paused){
 ship.move(dim.width,dim.height); // move the ship
 //move shots and remove dead shots
 for(int i=0;i<numShots;i++){
 shots[i].move(dim.width,dim.height);
 //removes shot if it has gone for too long
 //without hitting anything
 if(shots[i].getLifeLeft()<=0){
 //shifts all the next shots up one
 //space in the array
 deleteShot(i);
 i--; // move the outer loop back one so
 // the shot shifted up is not skipped
 }
 }
 //move asteroids and check for collisions
 updateAsteroids();

 if(shooting && ship.canShoot()){
 //add a shot on to the array
 shots[numShots]=ship.shoot();
 numShots++;
 }
 }

 repaint();
 try{
 endTime=System.currentTimeMillis();
 if(framePeriod-(endTime-startTime)>0)
 Thread.sleep(framePeriod-(endTime-startTime));
 }catch(InterruptedException e){
 }
 }
 }

private void deleteShot(int index){
 //delete shot and move all shots after it up in the array
 numShots--;
 for(int i=index;i<numShots;i++)
 shots[i]=shots[i+1];
 shots[numShots]=null;
 }

private void deleteAsteroid(int index){
 //delete asteroid and shift ones after it up in the array
 numAsteroids--;
 for(int i=index;i<numAsteroids;i++)
 asteroids[i]=asteroids[i+1];
 asteroids[numAsteroids]=null;
 }

private void addAsteroid(Asteroid ast){
 //adds the asteroid passed in to the end of the array
 asteroids[numAsteroids]=ast;
 numAsteroids++;
 }

private void updateAsteroids(){
 for(int i=0;i<numAsteroids;i++){
 // move each asteroid
 asteroids[i].move(dim.width,dim.height);
 //check for collisions with the ship, restart the
 //level if the ship gets hit
 if(asteroids[i].shipCollision(ship)){
 level--; //restart this level
 numAsteroids=0;
return;
 }
 //check for collisions with any of the shots
 for(int j=0;j<numShots;j++){
 if(asteroids[i].shotCollision(shots[j])){
 //if the shot hit an asteroid, delete the shot
 deleteShot(j);
 //split the asteroid up if needed
 if(asteroids[i].getHitsLeft()>1){
for(int k=0;k<asteroids[i].getNumSplit();
k++)
 addAsteroid(
 asteroids[i].createSplitAsteroid(
 minAstVel,maxAstVel));
 }
 //delete the original asteroid
 deleteAsteroid(i);
 j=numShots; //break out of inner loop - it has
 //already been hit, don't need to check
//for collision with other shots
 i--; //don't skip asteroid shifted back into
 //the deleted asteroid's position
 }
 }
 }
 }

public void keyPressed(KeyEvent e){
 if(e.getKeyCode()==KeyEvent.VK_ENTER){
 //These first two lines allow the asteroids to move
 //while the player chooses when to enter the game.
 //This happens when the player is starting a new life.
 if(!ship.isActive() && !paused)
 ship.setActive(true);
 else{
 paused=!paused; //enter is the pause button
 if(paused) // grays out the ship if paused
 ship.setActive(false);
 else
 ship.setActive(true);
 }
 }else if(paused || !ship.isActive()) //if the game is
 return; //paused or ship is inactive, do not respond
 //to the controls except for enter to unpause
 else if(e.getKeyCode()==KeyEvent.VK_UP)
 ship.setAccelerating(true);
 else if(e.getKeyCode()==KeyEvent.VK_LEFT)
 ship.setTurningLeft(true);
 else if(e.getKeyCode()==KeyEvent.VK_RIGHT)
 ship.setTurningRight(true);
 else if(e.getKeyCode()==KeyEvent.VK_SPACE)
 shooting=true; //Start shooting if ctrl is pushed
 }

public void keyReleased(KeyEvent e){
 if(e.getKeyCode()==KeyEvent.VK_UP)
 ship.setAccelerating(false);
 else if(e.getKeyCode()==KeyEvent.VK_LEFT)
 ship.setTurningLeft(false);
 else if(e.getKeyCode()==KeyEvent.VK_RIGHT)
 ship.setTurningRight(false);
 else if(e.getKeyCode()==KeyEvent.VK_SPACE)
 shooting=false;
 }

public void keyTyped(KeyEvent e){
 }
}





class Asteroid {
double x, y, xVelocity, yVelocity, radius;
int hitsLeft, numSplit;

public Asteroid(double x,double y,double radius,double minVelocity,
 double maxVelocity,int hitsLeft,int numSplit){
 this.x=x;
 this.y=y;
 this.radius=radius;
 this.hitsLeft=hitsLeft; //number of shots left to destroy it
 this.numSplit=numSplit; //number of smaller asteroids it
 //breaks up into when shot
 //calculates a random direction and a random
 //velocity between minVelocity and maxVelocity
 double vel=minVelocity + Math.random()*(maxVelocity-minVelocity),
 dir=2*Math.PI*Math.random(); // random direction
 xVelocity=vel*Math.cos(dir);
 yVelocity=vel*Math.sin(dir);
 }
public void move(int scrnWidth, int scrnHeight){
 x+=xVelocity; //move the asteroid
 y+=yVelocity;
 //wrap around code allowing the asteroid to go off the screen
 //to a distance equal to its radius before entering on the
 //other side. Otherwise, it would go halfway off the sceen,
 //then disappear and reappear halfway on the other side
 //of the screen.
 if(x<0-radius)
 x+=scrnWidth+2*radius;
 else if(x>scrnWidth+radius)
 x-=scrnWidth+2*radius;
 if(y<0-radius)
 y+=scrnHeight+2*radius;
 else if(y>scrnHeight+radius)
 y-=scrnHeight+2*radius;
 }

public void draw(Graphics g){
 g.setColor(Color.gray); // set color for the asteroid
 // draw the asteroid centered at (x,y)
 g.fillOval((int)(x-radius+.5),(int)(y-radius+.5),
 (int)(2*radius),(int)(2*radius));
 }

public Asteroid createSplitAsteroid(double minVelocity,
 double maxVelocity){
 //when this asteroid gets hit by a shot, this method is called
 //numSplit times by AsteroidsGame to create numSplit smaller
 //asteroids. Dividing the radius by sqrt(numSplit) makes the
 //sum of the areas taken up by the smaller asteroids equal to
 //the area of this asteroid. Each smaller asteroid has one
 //less hit left before being completely destroyed.
 return new Asteroid(x,y,radius/Math.sqrt(numSplit),
 minVelocity,maxVelocity,hitsLeft-1,numSplit);
 }

public boolean shipCollision(Ship ship){
 // Use the distance formula to check if the ship is touching this
 // asteroid: Distance^2 = (x1-x2)^2 + (y1-y2)^2 ("^" denotes
 // exponents). If the sum of the radii is greater than the
 // distance between the center of the ship and asteroid, they are
 // touching.
 // if (shipRadius + asteroidRadius)^2 > (x1-x2)^2 + (y1-y2)^2,
 // then they have collided.
 // It does not check for collisions if the ship is not active
 // (player is waiting to start a new life or the game is paused).
 if(Math.pow(radius+ship.getRadius(),2) >
Math.pow(ship.getX()-x,2) + Math.pow(ship.getY()-y,2)
&& ship.isActive())
 return true;
 return false;
 }
public boolean shotCollision(Shot shot){
 // Same idea as shipCollision, but using shotRadius = 0
 if(Math.pow(radius,2) > Math.pow(shot.getX()-x,2)+
 Math.pow(shot.getY()-y,2))
 return true;
 return false;
 }

public int getHitsLeft(){
 //used by AsteroidsGame to determine whether the asteroid should
 //be split up into smaller asteroids or destroyed completely.
 return hitsLeft;
 }

public int getNumSplit(){
 return numSplit;
 }
}




class Ship {
final double[] origXPts={14,-10,-6,-10},origYPts={0,-8,0,8},
 origFlameXPts={-6,-23,-6},origFlameYPts={-3,0,3};
final int radius=6;

double x, y, angle, xVelocity, yVelocity, acceleration,
 velocityDecay, rotationalSpeed;
boolean turningLeft, turningRight, accelerating, active;
int[] xPts, yPts, flameXPts, flameYPts;
int shotDelay, shotDelayLeft;

public Ship(double x, double y, double angle, double acceleration,
 double velocityDecay, double rotationalSpeed,
 int shotDelay){
 //this.x refers to the Ship's x, x refers to the x parameter
 this.x=x;
 this.y=y;
 this.angle=angle;
 this.acceleration=acceleration;
 this.velocityDecay=velocityDecay;
 this.rotationalSpeed=rotationalSpeed;
 xVelocity=0; // not moving
 yVelocity=0;
 turningLeft=false; // not turning
 turningRight=false;
 accelerating=false; // not accelerating
 active=false; // start off paused
 xPts=new int[4]; // allocate space for the arrays
 yPts=new int[4];
 flameXPts=new int[3];
 flameYPts=new int[3];
 this.shotDelay=shotDelay; // # of frames between shots
 shotDelayLeft=0; // ready to shoot
 }

public void draw(Graphics g){
 //rotate the points, translate them to the ship's location (by
 //adding x and y), then round them by adding .5 and casting them
 //as integers (which truncates any decimal place)
 if(accelerating && active){ // draw flame if accelerating
 for(int i=0;i<3;i++){
 flameXPts[i]=(int)(origFlameXPts[i]*Math.cos(angle)-
 origFlameYPts[i]*Math.sin(angle)+
 x+.5);
 flameYPts[i]=(int)(origFlameXPts[i]*Math.sin(angle)+
 origFlameYPts[i]*Math.cos(angle)+
 y+.5);
 }
 g.setColor(Color.red); //set color of flame
 g.fillPolygon(flameXPts,flameYPts,3); // 3 is # of points
 }
 //calculate the polgyon for the ship, then draw it
 for(int i=0;i<4;i++){
 xPts[i]=(int)(origXPts[i]*Math.cos(angle)- //rotate
 origYPts[i]*Math.sin(angle)+
 x+.5); //translate and round
 yPts[i]=(int)(origXPts[i]*Math.sin(angle)+ //rotate
 origYPts[i]*Math.cos(angle)+
 y+.5); //translate and round
 }
 if(active) // active means game is running (not paused)
 g.setColor(Color.white);
 else // draw the ship dark gray if the game is paused
 g.setColor(Color.darkGray);
 g.fillPolygon(xPts,yPts,4); // 4 is number of points
 }

public void move(int scrnWidth, int scrnHeight){
 if(shotDelayLeft>0) //move() is called every frame that the game
 shotDelayLeft--; //is run; this ticks down the shot delay
 if(turningLeft) //this is backwards from typical polar coodinates
 angle-=rotationalSpeed; //because positive y is downward.
 if(turningRight) //Because of that, adding to the angle is
 angle+=rotationalSpeed; //rotating clockwise (to the right)
 if(angle>(2*Math.PI)) //Keep angle within bounds of 0 to 2*PI
 angle-=(2*Math.PI);
 else if(angle<0)
 angle+=(2*Math.PI);
 if(accelerating){ //adds accel to velocity in direction pointed
 //calculates components of accel and adds them to velocity
 xVelocity+=acceleration*Math.cos(angle);
 yVelocity+=acceleration*Math.sin(angle);
 }
 x+=xVelocity; //move the ship by adding velocity to position
 y+=yVelocity;
 xVelocity*=velocityDecay; //slows ship down by percentages
 yVelocity*=velocityDecay; //(velDecay should be between 0 and 1)
 if(x<0) //wrap the ship around to the opposite side of the screen
 x+=scrnWidth; //when it goes out of the screen's bounds
 else if(x>scrnWidth)
 x-=scrnWidth;
 if(y<0)
 y+=scrnHeight;
 else if(y>scrnHeight)
 y-=scrnHeight;
 }

public void setAccelerating(boolean accelerating){
 this.accelerating=accelerating;
 }

public void setTurningLeft(boolean turningLeft){
 this.turningLeft=turningLeft;
 }

public void setTurningRight(boolean turningRight){
 this.turningRight=turningRight;
 }

public double getX(){
 return x;
 }

public double getY(){
 return y;
 }

public double getRadius(){
 return radius;
 }

public void setActive(boolean active){
 this.active=active;
 }

public boolean isActive(){
 return active;
 }

public boolean canShoot(){
 if(shotDelayLeft>0) //checks to see if the ship is ready to
 return false; //shoot again yet or needs to wait longer
 else
 return true;
 }
public Shot shoot(){
 shotDelayLeft=shotDelay; //set delay till next shot can be fired
 //a life of 40 makes the shot travel about the width of the
 //screen before disappearing
 return new Shot(x,y,angle,xVelocity,yVelocity,40);
 } }




class Shot {
final double shotSpeed=12;
double x,y,xVelocity,yVelocity;
int lifeLeft;

public Shot(double x, double y, double angle, double shipXVel,
 double shipYVel, int lifeLeft){
 this.x=x;
 this.y=y;
 // add the velocity of the ship to the velocity the shot velocity
 // (so the shot's velocity is relative to the ship's)
 xVelocity=shotSpeed*Math.cos(angle)+shipXVel;
 yVelocity=shotSpeed*Math.sin(angle)+shipYVel;
 // the number of frames the shot will last for before
 // disappearing if it doesn't hit anything
 this.lifeLeft=lifeLeft;
 }

public void move(int scrnWidth, int scrnHeight){
 lifeLeft--; // used to make shot disappear if it goes too long
 // without hitting anything
 x+=xVelocity; // move the shot
 y+=yVelocity;
 if(x<0) // wrap the shot around to the opposite side of the
 x+=scrnWidth; // screen if needed
 else if(x>scrnWidth)
 x-=scrnWidth;
 if(y<0)
 y+=scrnHeight;
 else if(y>scrnHeight)
 y-=scrnHeight;
 }

public void draw(Graphics g){
 g.setColor(Color.yellow); //set shot color
 //draw circle of radius 3 centered at the closest point
 //with integer coordinates (.5 added to x-1 and y-1 for rounding)
 g.fillOval((int)(x-.5), (int)(y-.5), 3, 3);
 }

public double getX(){
 return x;
 }

public double getY(){
 return y;
 }

public int getLifeLeft(){
 return lifeLeft;
 }
}