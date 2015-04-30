/*****************************************************************************************
 * Team Indecision
 * Source code from https://github.com/academicode/app-simple-tip-calculator/tree/session-7
 * https://www.youtube.com/watch?v=Z3jzIYkxB1s
 *****************************************************************************************/
package org.academicode.unitconverter;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

/*  the result calculated in Main.java is printed on the screen. The frontend of this file is
  * implemented in result.xml */
public class Result extends Activity 
{
	//Initializes TextViews to display the conversion result
	private TextView conTextView;


	//Initializes a button
	private Button finished;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		
		//activity's front end  in result.xml
		setContentView(R.layout.result);
		
		/*Set the texts views so they display according to the parameters in result.xml*/
        conTextView = (TextView) findViewById(R.id.finalamount);

		//Runs the method to use the values calculated in the Main Activity class
		initializeTextViews();
		
		//Initializes button described in result.xml
		finished = (Button) findViewById(R.id.confirm);
		
		//Sets an onClickListener for the finished button
		finished.setOnClickListener(new OnClickListener() 
		{
			@Override
			//If clicked, call the finish method
			public void onClick(View v) 
			{
				//Finish ends the current activity and goes back to the activity described in Main.java
				finish();
			}
		});
	}

	//finalizes the result that needs to be printed on the screen
	private void initializeTextViews()
	{
		//converts values from Main.java into string
		String tip = getIntent().getExtras().getString(Main.TAG_converg);

		String currentTipText = conTextView.getText().toString();

		currentTipText = currentTipText + tip ;

		//Sets the texts to display the values
        conTextView.setText(currentTipText);

	}
}
