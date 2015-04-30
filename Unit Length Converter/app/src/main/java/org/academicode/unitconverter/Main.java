/*****************************************************************************************
 * Team Indecision for use in ENG EC 327
 * Source code from https://github.com/academicode/app-simple-tip-calculator/tree/session-7
 * Tutorial https://www.youtube.com/watch?v=Z3jzIYkxB1s (where the source is from)
 *****************************************************************************************/
package org.academicode.unitconverter;

/*This java file will run after the user goes though the opening screen. This file includes
mathematical calculations required for unit conversion. The frontend of this file is implemented in
main.xml. Here the buttons and text boxes declared in main.xml are implemented and the calculations
are performed. The result is then pass to Result.java*/

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;


public class Main extends Activity implements OnClickListener
{
	//Declaring and setting string variables
	private static final String TAG_DEBUG = Main.class.getName();
	public static final String TAG_converg= "tag";

	String unit;//string stores the new units after conversion
	//Creating the EditText object
	private EditText et;
	
	//Creating buttons
	private Button buttoncmtoin;
	private Button buttonintocm;
	private Button buttonfttom;
	private Button buttonmtoft;
    private Button buttonydtom;
    private Button buttonmtoyd;
	@Override

	protected void onCreate(Bundle savedInstanceState) 
	{

		super.onCreate(savedInstanceState);

		setContentView(R.layout.main);
		

		et = (EditText) findViewById(R.id.editText1);


		//assigning buttons to their corresponding ids
		buttoncmtoin = (Button) findViewById(R.id.cmtoin);
		buttonintocm = (Button) findViewById(R.id.intocm);
		buttonfttom = (Button) findViewById(R.id.fttom);
		buttonmtoft = (Button) findViewById(R.id.mtoft);
        buttonydtom = (Button) findViewById(R.id.ydtom);
        buttonmtoyd = (Button) findViewById(R.id.mtoyd);

		// onClickListeners allows the button to carryout their functions

		buttoncmtoin.setOnClickListener(this);
		buttonintocm.setOnClickListener(this);
        buttonfttom.setOnClickListener(this);
        buttonmtoft.setOnClickListener(this);
        buttonydtom.setOnClickListener(this);
        buttonmtoyd.setOnClickListener(this);
	}

	@Override


    //onClick is called when a button is pressed.
 	public void onClick(View v)
	{
		double conversion = 0.0d;
		
		//The switch statements grab the id values of the button pressed and calculates the conversion
        //and assigns units to the corresponding conversion
		switch(v.getId()){
		
			case R.id.cmtoin :
			{
				conversion = 0.393701d;
                unit="in";
				break;
			}
			case R.id.intocm :
			{
                conversion = 2.54d;
                unit="cm";
				break;
			}
			case R.id.fttom :
			{
                conversion = 0.3048d;
                unit="m";
				break;
			}
            case R.id.mtoft:
            {
                conversion = 3.28084d;
                unit="ft";
                break;
            }
            case R.id.ydtom:
            {
                conversion= 0.9144d;
                unit="m";
                break;
            }
            case R.id.mtoyd :
            {
                conversion = 1.09361d;
                unit="yd";
                break;
            }
			default :
			{
				break;
			}
		}
		
		//reads the text entered into the text box
		String text = et.getText().toString();
		
		//If the user tried to proceed without entering a value--shows an error
		if(text.equals(""))
		{

			Toast.makeText(Main.this, getResources().getString(R.string.error_et), Toast.LENGTH_LONG).show();

			return;
		}
		

        //if user enters the correct input, the input - conversion is passed to function to
        // turn the string into a double to use is a different activity
		launchResultActivity(Double.parseDouble(et.getText().toString()), conversion);
	}

	private void launchResultActivity(double total, double conversion)
	{
		double finalcon = total * conversion;

        /*intent loads the activities so that the next activity
        can be launched. Initializes an Intent named resultActivity and
        passes (Main.this,Result.class) to it*/
  		Intent resultActivity = new Intent(Main.this, Result.class);
		

		resultActivity.putExtra(TAG_converg, finalcon + unit);


		//Launches the new activity
		startActivity(resultActivity);
	}
}
