<!-----

You have some errors, warnings, or alerts. If you are using reckless mode, turn it off to see inline alerts.
* ERRORs: 0
* WARNINGs: 0
* ALERTS: 1

Conversion time: 1.155 seconds.


Using this Markdown file:

1. Paste this output into your source file.
2. See the notes and action items below regarding this conversion run.
3. Check the rendered output (headings, lists, code blocks, tables) for proper
   formatting and use a linkchecker before you publish this page.

Conversion notes:

* Docs to Markdown version 1.0β33
* Fri Nov 11 2022 04:35:49 GMT-0800 (PST)
* Source doc: AVTokyo 2022 event badge readme
* This document has images: check for >>>>>  gd2md-html alert:  inline image link in generated source and store images to your server. NOTE: Images in exported zip file from Google Docs may not appear in  the same order as they do in your doc. Please check the images!

----->


<p style="color: red; font-weight: bold">>>>>>  gd2md-html alert:  ERRORs: 0; WARNINGs: 0; ALERTS: 1.</p>
<ul style="color: red; font-weight: bold"><li>See top comment block for details on ERRORs and WARNINGs. <li>In the converted Markdown or HTML, search for inline alerts that start with >>>>>  gd2md-html alert:  for specific instances that need correction.</ul>

<p style="color: red; font-weight: bold">Links to alert messages:</p><a href="#gdcalert1">alert1</a>

<p style="color: red; font-weight: bold">>>>>> PLEASE check and correct alert issues and delete this message and the inline alerts.<hr></p>



## AVTokyo 2022 event badge


### MQTT enabled, Hacker tools, Sensors and Games on ESP32



<p id="gdcalert1" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/image1.png). Store image on your image server and adjust path/filename/extension if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert2">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](Images/promo/AFT2022badge_TK.jpg "image_tooltip")


The AVTokyo 2022 event attendee badge was designed and produced by VPCAsia. Our vision was to provide a useful tool for the attendee not only at the event, but far into the future.

It is based on an 8MB ESP32 with 280x240 pixel TFT full color display (sourced from smart watches). Additionally, it includes a joystick (4 directions plus “press to select”), two game buttons (B and A), and two sensors (Temperature/Humidity and Accelerometer). Finally, the device is fully battery powered, relying on an 18650 style Li-ion battery with associated protections and USB charging.

The software runs a variety of open source Arduino libraries. In particular, we utilized tcMenu and TFT_eSPI to provide all of our software UI, and Adafruit libraries to manage sensors and MQTT (Message Queue, Telemetry Transport).

While WiFi is connected and MQTT messaging is enabled, the badge will send Temperature, Humidity and Battery Voltage measurements to an MQTT server every 5 seconds (the interval is redefinable). Additionally, when you obtain a new best personal score on any game, the new score will be queued to be sent in the next MQTT post. While gaming, MQTT posting is temporarily disabled, in order to provide the best gaming experience.


## Tour of the Board


## Fixes to the production version


## About the Server

On our server side, we used a variety of services. It all starts by receiving the MQTT message in the MQTT server. From here, messages are written to a database using some custom Java.

— more description from Chris —

We can also read messages from the MQTT server using NodeRed. 
