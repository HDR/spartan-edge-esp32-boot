/*
 * 01LoadDefaultBitstream
 *  
 * loading the default Bitstream
 *
 * The MIT License (MIT)
 * Copyright (C) 2019  Seeed Technology Co.,Ltd.
 * 
 * Additions made to have a small webserver for updating
 * the bitstream by zwenergy.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SD_MMC.h>

// include the library:
#include <spartan-edge-esp32-boot.h>

// initialize the spartan_edge_esp32_boot library
spartan_edge_esp32_boot esp32Cla;

// the bitstream name which we loading
#define LOADING_DEFAULT_FIEE "/overlay/default.bit"

// WiFi defs.
const char* ssid = "gbaHD";
const char* password = "gbahdwifi";
WebServer server( 80 );

// We need to store the uploaded file.
File fsUpload;

// HTML stuff.
const char* main =
"<h1>gbaHD FW Update</h1><p>\
<p>Upload bitstream below.</p>\
<form method='post' enctype='multipart/form-data'>\
<input type='file' name='name'>\
<input class='button' type='submit' value='Upload to gbaHD'>\
</form>";

// Server functions.
void handleRoot() {
  Serial.println( "Handling root GET" );
  // Create main page.
  String mainPage( main );
  // Send the main page.
  server.send( 200, "text/html", mainPage );
}

void handle404() {
  server.send( 404, "text/plain", "Not found." );
}

void sendOK() {
  server.send( 200 );
}

void handleFileUpload() {
  // TBD: Assert size.
  
  // Receive the file and store it into the file system.
  // The SD card should be already mounted.
  HTTPUpload& upload = server.upload();
  if ( upload.status == UPLOAD_FILE_START ) {
    // Open the file to write.
    fsUpload = SD_MMC.open( LOADING_DEFAULT_FIEE, "w" );
  } else if ( upload.status == UPLOAD_FILE_WRITE ) {
    if ( fsUpload ) {
      fsUpload.write( upload.buf, upload.currentSize );
    }
  } else if ( UPLOAD_FILE_END ) {
    if ( fsUpload ) {
      fsUpload.close();
      Serial.println( "Received file" );
      // Send a response.
      server.sendHeader( "Location", "/" );
      server.send( 303 );
    } else {
      server.send( 500, "text/plain", "500: Error creating file." );
    }
  }
}


// the setup routine runs once when you press reset:
void setup() {
  Serial.begin( 115200 );
  
  // initialization 
  esp32Cla.begin();

  // XFPGA pin Initialize
  esp32Cla.xfpgaGPIOInit();

  // loading the bitstream
  esp32Cla.xlibsSstream(LOADING_DEFAULT_FIEE);

  // Setup the WiFi.
  WiFi.softAP( ssid, password );
  IPAddress ip = WiFi.softAPIP();
  Serial.print( "IP adress: " );
  Serial.println( ip );

  // Set up DNS.
  if ( !MDNS.begin( "gbahd" ) ) {
    Serial.println( "Error setting up DNS" );
  }

  // Handle root access GET.
  server.on( "/", HTTP_GET, handleRoot );
  
  // Handle the upload.
  server.on( "/", HTTP_POST, sendOK, handleFileUpload );

  // Handle everything else.
  server.onNotFound( handle404 );

  server.begin();
}

// the loop routine runs over and over again forever:
void loop() {
  // Check for clients.
  server.handleClient();
}
