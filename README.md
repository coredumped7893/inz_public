# inz_public
Autonomiczne sterowanie dronami z wykorzystaniem technik rozpoznawania obrazów i map terenu


KOMPILOWANIE PROJEKTU

Wymagania:   
•	System: GNU/Linux – testowane na Ubuntu 20.04 oraz Raspbian 10 (buster)  
•	Kompilator GCC i G++ – testowane były wersje Raspbian 8.3.0 oraz Ubuntu 9.3.0  
•	make  
•	pkg-config  
Wymagane biblioteki:  
Systemowe: m, pthread  
Zewnętrzne:   
•	socket, c-utils – po skompilowaniu powinny zostać umieszczone w folderze FlightSystem/lib/{local lub PI}/  
•	Opencv 4.5.* - wraz z modułami: FFMPEG, GStreamer,  
o	pkg-config powinien wykrywać instalację pod nazwą „opencv4”. W przeciwnym wypadku wymagana będzie modyfikacja pliku Makefile  

Kompilowanie (będąc w folderze FlightSystem): make   
Po udanej kompilacji, pliki wykonywalne zostaną stworzone w folderze FilghtSystem/build o nazwach: FS_exec i FS_exec_img  
Uruchomienie (będąc w folderze FlightSystem): make runr1 lub make runr2  
	runr1 – kontrola lotu, komunikacja z symulatorem.  
	runr2 – analiza obrazu z kamery, wykrywanie kolizji z terenem na podstawie map  

Uruchomienie testów: make tests TEST=1  
Wygenerowanie dokumentacji: make doxygen  

