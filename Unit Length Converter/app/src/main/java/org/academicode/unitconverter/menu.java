package org.academicode.unitconverter;

/**
 * Created by punker422 on 4/29/15.
 */
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.content.Intent;

import android.widget.TextView;

/* File creates the opening screen, first file that will run. This screen consists of a background picture, some text
* and a button, which when pressed will allow user to proceed with the app. The front end of this
* java file is described in menu.xml*/
public class menu extends Activity
{


    private Button Enter;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        //How this activity actually looks is set inside result.xml
        setContentView(R.layout.menu);

        //Initializes button to the parameters in menu.xml
       Enter = (Button) findViewById(R.id.button);

        //Sets an onClickListener for the Enter button
       Enter.setOnClickListener(new View.OnClickListener()
        {
            @Override
            //If clicked, call the finish method
            public void onClick(View v)
            {
                //Redirects the user to the Main activity
                Intent resultActivity = new Intent(menu.this, Main.class);
                startActivity(resultActivity);
            }
        });
    }
}
