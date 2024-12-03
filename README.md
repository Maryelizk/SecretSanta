# SecretSanta
Automated Secret Santa name picker selects two gift recipients for each participant using Arduino and RFIDs. 


This project uses an Arduino Nano, RFID reader, RFID tags (I used ones designed for laundry and shaped like buttons), and a custom designed 3D printed housing and display that fits inside a sturdy cardboard gift box. I used green, white, and red LEDs to illuminate the names and an ON/ON toggle connected on only one side to engage a 9v battery. 

3D Files
The housing is designed in four parts. 
1. Boardside - This is the largest piece, it has a top face that should print facedown with ovals cut out for light to shine through. This allows the name plate to be swapped in and out. The inside of the boardside piece has standoffs for the boards, fins to block light spill from the LEDs behind the names, and a collar that provides the spacing for the battery holder.
2. BatterySide - This is a disk with bands that hold a 9v battery. I used the lip of the boardside piece to glue in magnets that attach to the flat battery side piece. The original idea was this piece would friction fit with the collar lip but that never worked out.
3. thinNames - This is the piece that includes all the names. It is printed by having the printer lay down one color that will be the background color for the names (ideally this will be slightly translucent but as long as it's printed thin it should be fine) then the filament should be swapped so the rest of the print is in a contrasting color. Names were made using Cuttle.xyz loosely following this tutorial https://www.youtube.com/watch?v=i0Oduk7Lc60.
4. Lip and Lip Catch - The lip pieces are designed to wedge into my box, which was ~8mm larger than my build plate. The initial design used only the three lip pieces but the lip catch can be added to connect to the power on/off toggle switch. This doesn't work very well because the piece slides off after changing the toggle position once. But it's good for when you want turning it on to not involve completely disassembling the box. 


Considerations and why they were discarded:
- Printing the entire thing in one go. The names look best when printed flat against the build plate. If they need to be printed as a later layer without supports the filament will look stringy when the light shines through it.
- I tried many different ways to smooth the top of the name plate and still settled on an option that looks a bit under-extruded. Area for improvement
- The Arduino does have capabilities to store the pairings when turned-off. This could be useful if someone forgets their pair or so our referee can make connections when different gifters want to coordinate. I thought it was unlikely we'd use that functionality and more likely we'd cheat, so I didn't include it. It would also require implementing a way to reset the pairings for a new round, which would need to happen a lot because I like showing off the device
- Hard coding the RFIDs instead of having a registration process each time. This could be done in the future, but the sign in ceremony is fun. I did consider faking the sign in ceremony so that it's actually not necessary but the first time you tap your hard it lights up your name with the fanfare anyway. This could be an option in the future.


![image](https://github.com/user-attachments/assets/36d9b120-37cc-4bd4-8605-f35efd667fc8)
https://photos.app.goo.gl/kjEQuw1Hfb5g27JZ6
