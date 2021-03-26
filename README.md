# MaterialScannerHsH

Zunächst muss Visual Studio 2019 installiert werden. Dabei bitte das Workload "Desktop development with C++" hinzufügen.
Bitte auch das englische Sprachpaket benutzen. Anschließend, oder davor, bitte Git installieren.

Zuerst muss das Repository cloned werden. Anschließend müssen die Submodule initialisiert werden.

git clone https://github.com/akuzminykh/MaterialScannerHsH.git
cd .\MaterialScannerHsH\
git submodule update --init

Jetzt sollte Visual Studio gestartet werden. Dabei bitte im Startfenster "Continue without code" klicken.
Nun bitte "File -> New -> Project From Existing Code..." klicken. Als Typ sollte "Visual C++" ausgewählt sein.
Im nächsten Fesnter dann "Project file location:" auf den Ordner "MaterialScannerHsH" setzen.
Der Projektname sollte "MaterialScannerHsH" lauten. Im nächsten Fenster das Dropdown von "Windows application project"
auf "Console application project" setzen. Anschließend zweimal "Next >" und schließlich "Finish".

Im "Solution Explorer" sollte nun das Projekt zu sehen sein. Im Ordner "Source Files" liegt z. B. "main.cpp".
Bitte die Datei "example.cpp" aus dem Projekt nehmen ("remove" ohne Löschen).

Oben sind zwei Dropdowns, die jeweils auf "Debug" und auf "x86" eingestellt sind.
Diese müssen jeweils auf "Release" und auf "x64" gesetzt werden. Jetzt muss der C++17 Compiler für das Projekt gesetzt werden.
Dazu Rechtsklick auf das Projekt ("MaterialScannerHsH"), dann nach "Properties -> Configuration Properties -> General" navigieren.
Dort muss nun für "C++ Language Standard" der Standard "ISO C++17 Standard (std:c++17)" ausgewählt werden. Hier "Apply" nicht vergessen.

Das Projekt ist nun eingerichtet. Das Programm hat OpenImageIO als Dependency. Für die Installation empfehle ich "vcpkg".
Das ist ein Paketmanager für Libraries. Also bitte "https://github.com/microsoft/vcpkg" öffnen und den "Quick Start" machen.
Diese Befehle sollten ausgeführt werden:

vcpkg install openimageio --triplet x64-windows --clean-after-build
vcpkg integrate install

Mögliche Probleme:
- Command "vcpkg" kann nicht gefunden werden. Lösung: Das ist eine ".exe"-Datei. Also den Ordner "vcpkg" in
  der Umgebungsvariablen "Path" aufnehmen, oder beim Aufruf den ganzen Pfad zur "vcpkg.exe" angeben.
- Fehler beim Ausführen. Lösung: Ggf. müssen einige Komponenten in Visual Studio hinzugefügt werden. Bitte die Fehlermeldung lesen.
  Es könnten z. B. "C++ ATL for latest v142 build tools (x86 & x64)" und/oder "C++ MFC for latest v142 build tools (x86 & x64)" fehlen.
- Es wird eine alte Windows 10 SDK benutzt. Lösung: Die neueste Komponente zu Visual Studio hinzufügen, z. B. "Windows 10 SDK (10.0.19041.0)".

An dieser Stelle sollte Visual Studio neugestartet und "Build -> Build Solution" ausgeführt werden.
Im Ordner "x64/Release" sollte nun "MaterialScannerHsH.exe" zu finden sein. Zum Ausprobieren, im Ordner diesen Befehl ausführen:

.\MaterialScannerHsH.exe ..\..\datasets\object1\ test.jpg 15

Im Ordner sollte nun ein Bild zu finden sein, dass die Normalmap für den angegebenen Datensatz darstellt.
Wenn das der Fall ist, dann hast Du diesen Guide erfolgreich überlebt! :)

----

Es gibt sicher viele andere Möglichkeiten sich das Projekt aufzusetzen und das Programm zu kompilieren.
Dieser Guide ist nur eine kleine Anleitung dazu, wie man es machen kann.

Wenn etwas nicht funktioniert, dann bitte die Fehlermeldungen lesen und das Internet benutzen.
Wenn etwas gar nicht geht oder Du stecken bleibst, dann E-Mail an: alexander.kuzminykh@stud.hs-hannover.de