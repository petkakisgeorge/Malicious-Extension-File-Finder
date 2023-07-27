# Malicious-Extension-File-Finder
A Windows Searching Program in C++ (portable) that identifies potentially harmful file extensions on a Windows machine.
<!-- Image displayed at 70% of its original size -->
<img src="https://github.com/petkakisgeorge/Malicious-Extension-File-Finder/blob/main/resources/example_images/GUI%20app.png?raw=true" alt="Screenshot of GUI app" width="70%">

**Introduction:** \
Malicious Extension File Finder is an advanced C++ tool that empowers Blue Teams and incident responders to efficiently and proactively detect potentially harmful files on Windows machines. Combining a Graphical User Interface (GUI) and Console Application, the tool offers a user-friendly experience while conducting comprehensive searches for files with malicious extensions, hidden double extensions, specific filenames, and a wide range of predefined malicious extensions like batch files, Office Macros, VBScript, and PowerShell Scripts. Furthermore, users can customize the search criteria to include other extensions relevant to their security needs.

**User-Friendly Interface:** \
The integration of a GUI and Console Application allows users of varying technical expertise to effectively utilize the tool. The GUI provides an intuitive interface for easy configuration, file selection, and customization of search parameters. Users can specify the target directory and define the criteria for identifying potentially malicious files. The Console Application complements the GUI by offering more advanced users the flexibility to automate searches and integrate the tool into existing security workflows.
<!-- Image displayed at 70% of its original size -->
<img src="https://github.com/petkakisgeorge/Malicious-Extension-File-Finder/blob/main/resources/example_images/search%20console.png?raw=true" alt="Screenshot of Console app" width="70%">

**Comprehensive Search Capabilities:** \
Malicious Extension File Finder's key strength lies in its comprehensive search capabilities. By scanning for files with multiple extensions, including hidden double extensions like "test.doc.exe," it identifies suspicious files that may attempt to masquerade as innocuous documents or executables. Moreover, the ability to search for specific filenames streamlines the process of identifying known threats or unauthorized files.

**Predefined and Custom Malicious Extensions:** \
The tool's built-in database of predefined malicious extensions, encompassing batch files, Office Macros, VBScript, and PowerShell Scripts, enhances its threat detection capabilities. However, recognizing the ever-changing threat landscape, users can define their own malicious extensions relevant to their specific security requirements.
<!-- Image displayed at 70% of its original size -->
<img src="https://github.com/petkakisgeorge/Malicious-Extension-File-Finder/blob/main/resources/example_images/app_use_case.png?raw=true" alt="Screenshot of app use case" width="70%">

**Proactive Threat Detection:**  \
By promptly identifying potential threats, the Malicious Extension File Finder enables Blue Teams and incident responders to take proactive measures to safeguard their systems and data. Early detection allows for immediate investigation and containment, minimizing the risk of significant data breaches, unauthorized access, or further propagation of malware within the network.

**Installation and Usage:**
- Download the zip from releases.
- Extract it inside any directory you want (portable).\
**Run the app:**
- Run from GUI (double click **MaliciousFinder.exe**)
- Run from cmd directly (given the args by yourself)\
Example: **"search console.exe" C:\Users\admin\Desktop\ C:\ 1 1 1 1 0 0 1 empty empty**

**In case you want to compile it..**\
**Compile commands of GUI app (maliciousFinder.cpp)**:
- windres resource.rc -o resources.o
- g++ -static -o maliciousFinder.exe maliciousFinder.cpp -lole32 -lgdi32 resources.o

**Compile commands of Search/Console app (search console.cpp)**:
- g++ -static -o "search console.exe" "search console.cpp"

