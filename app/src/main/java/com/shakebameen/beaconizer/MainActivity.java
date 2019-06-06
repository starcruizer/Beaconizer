package com.shakebameen.beaconizer;


import android.app.Activity;
import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

import androidx.annotation.Nullable;

import com.google.firebase.storage.FirebaseStorage;
import com.google.firebase.storage.StorageReference;

import org.altbeacon.beacon.Beacon;
import org.altbeacon.beacon.BeaconConsumer;
import org.altbeacon.beacon.BeaconManager;
import org.altbeacon.beacon.BeaconParser;
import org.altbeacon.beacon.Identifier;
import org.altbeacon.beacon.RangeNotifier;
import org.altbeacon.beacon.Region;

import java.util.ArrayList;
import java.util.Collection;

public class MainActivity extends Activity implements BeaconConsumer, RangeNotifier {

    private final String TAG = "Beaconizer";
    private BeaconManager mBeaconManager;
    private float BeaconDistance;
    private ListView listView ;
    private ArrayList<String> stringArrayList = new ArrayList<>();
    private ArrayAdapter<String> arrayAdapter ;


    //Firebase Reference
    FirebaseStorage FirebaseStorageRef = FirebaseStorage.getInstance();

    //Image reference
    StorageReference storageRef = FirebaseStorageRef.getReference();
    StorageReference imageStorageRef = storageRef.child("AdvertisementImages");
    public StorageReference Adidas = imageStorageRef.child("Adidas01.jpg");
    public StorageReference Nike   = imageStorageRef.child("Nike01.jpg");
    public StorageReference Puma = imageStorageRef.child("Puma01.jpg");

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        arrayAdapter = new ArrayAdapter<>(getApplicationContext(),
                android.R.layout.simple_list_item_1, stringArrayList);
        listView = (ListView) findViewById(R.id.lstView);

    }

    @Override
    public void onResume() {
        super.onResume();
        mBeaconManager = BeaconManager.getInstanceForApplication(this.getApplicationContext());
        // Detect the main Eddystone-UID frame:
        mBeaconManager.getBeaconParsers().add(new BeaconParser().
                setBeaconLayout(BeaconParser.EDDYSTONE_UID_LAYOUT));
        // Detect the telemetry Eddystone-TLM frame:
        mBeaconManager.getBeaconParsers().add(new BeaconParser().
                setBeaconLayout(BeaconParser.EDDYSTONE_TLM_LAYOUT));
        mBeaconManager.bind(this);


    }

    public void onBeaconServiceConnect() {

        Region region = new Region("all-beacons-region", null, null, null);
        try {
            mBeaconManager.startRangingBeaconsInRegion(region);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        mBeaconManager.addRangeNotifier(this);

    }

    @Override
    public void didRangeBeaconsInRegion(Collection<Beacon> beacons, Region region) {
        for (Beacon beacon: beacons) {
            if (beacon.getServiceUuid() == 0xfeaa && beacon.getBeaconTypeCode() == 0x00) {

                // This is a Eddystone-UID frame

                Identifier namespaceId = beacon.getId1();
                Identifier instanceId = beacon.getId2();
                BeaconDistance = (float) beacon.getDistance();
                Log.d(TAG, "I see a beacon transmitting namespace id: "+namespaceId+
                        " and instance id: "+instanceId+
                        " approximately "+BeaconDistance+" meters away.");


                //adding beacon to the array
                stringArrayList.add("UUID - "+namespaceId+instanceId);
                arrayAdapter.notifyDataSetChanged();

                //Toast.makeText(this, "hello", Toast.LENGTH_SHORT).show();

                // Do we have telemetry data?

                if (beacon.getExtraDataFields().size() > 0) {
                    long telemetryVersion = beacon.getExtraDataFields().get(0);
                    long batteryMilliVolts = beacon.getExtraDataFields().get(1);
                    long pduCount = beacon.getExtraDataFields().get(3);
                    long uptime = beacon.getExtraDataFields().get(4);

                    Log.d(TAG, "The above beacon is sending telemetry version "+telemetryVersion+
                            ", has " +
                            "been up for : "+uptime+" seconds"+
                            ", has a battery level of "+batteryMilliVolts+" mV"+
                            ", and has transmitted "+pduCount+" advertisements.");

                }
            }
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        mBeaconManager.unbind(this);
    }
}
