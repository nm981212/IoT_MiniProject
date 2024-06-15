package net.kccistc.socketclient;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.ToggleButton;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import java.text.SimpleDateFormat;
import java.util.Date;

public class MainActivity extends AppCompatActivity {
    static MainHandler mainHandler;
    ClientThread clientThread;
    ToggleButton toggleButtonConnect;
    Button buttonSend;
    TextView textViewRecv;
    ScrollView scrollViewRecv;
    SimpleDateFormat dateFormat = new SimpleDateFormat("yy.MM.dd HH:mm:ss");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        EditText editTextIp = findViewById(R.id.editTextTextIp);
        EditText editTextPort = findViewById(R.id.editTextTextPort);
        scrollViewRecv = findViewById(R.id.scrollViewRecv);
        textViewRecv = findViewById(R.id.textViewRecv);
        toggleButtonConnect = findViewById(R.id.toggleButtonConnect);
        EditText editTextSend = findViewById(R.id.editTextTextSend);
        buttonSend = findViewById(R.id.buttonSend);
        buttonSend.setEnabled(false);
        toggleButtonConnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(toggleButtonConnect.isChecked()) {
                    String Ip = editTextIp.getText().toString();
                    int Port =  Integer.parseInt(editTextPort.getText().toString());
                    clientThread = new ClientThread(Ip,Port);
                    clientThread.start();
                    toggleButtonConnect.setChecked(false);
                } else {
                    clientThread.stopClient();
                    buttonSend.setEnabled(false);
//                    toggleButtonConnect.setChecked(true);
                }
            }
        });
        buttonSend.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String strSend = editTextSend.getText().toString();
                clientThread.sendData(strSend);
                editTextSend.setText("");
            }
        });
        mainHandler = new MainHandler();
    }
    class MainHandler extends Handler {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            Bundle bundle = msg.getData();
            String data = bundle.getString("msg");
            Log.d("MainHandler",data);
            if(data.indexOf("New connect") != -1) {
                toggleButtonConnect.setChecked(true);
                buttonSend.setEnabled(true);
                return;
            }
            Date date = new Date();
            String strDate = dateFormat.format(date);
            strDate = strDate + " " + data;
            textViewRecv.append(strDate);
            scrollViewRecv.fullScroll(View.FOCUS_DOWN);
        }
    }
}
