package com.shakebameen.beaconizer;

import com.google.firebase.storage.FirebaseStorage;
import com.google.firebase.storage.StorageReference;

class StorageActivity {

    //Firebase Reference
    FirebaseStorage FirebaseStorageRef = FirebaseStorage.getInstance();

    //Image reference
    StorageReference storageRef = FirebaseStorageRef.getReference();
    StorageReference imageStorageRef = storageRef.child("AdvertisementImages");
}
