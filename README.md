![](assets/images/i001.png?raw=true "CGITester")

A desktop application developed in the Go programming language. The purpose of this application is to facilitate the testing of 32-bit and 64-bit CGI (Common Gateway Interface) scripts/code designed for Windows and Linux operating systems.

### Information required for the execution of CGI scripts/code

#### Description:

A brief description of your CGI script/code. This information is optional.

#### Executable file:

Path to the CGI script/code file or to the program that will execute the CGI script/code on your operating system. Examples:

```
C:\CGITester\c-CGI\DBF-Viewer-2.exe

C:\Users\xpto\AppData\Local\Programs\Python\Python315\pythonw.exe

C:\Program Files\R\R-4.6.1\bin\Rscript.exe

/home/CGITester/c-CGI/DBF-Viewer.exe

/usr/local/bin/python3.15

/usr/bin/Rscript
```

#### Initial HTML file:

Path to the HTML file containing the FORM structured for calling the CGI script/code. This information is optional. Important detail: The FORM's ACTION property must always be set to "/registered-cgi/". Examples:

```
<form action="/registered-cgi/" method="GET">
	<select id="S1" name="S1">
		<option value="1">A</option>
		<option value="2">B</option>
		<option value="3">C</option>
	</select>
	<input type="submit" value="Send">
</form>

<form action="/registered-cgi/" method="POST" enctype="multipart/form-data">
	<input type="file" id="F1" name="F1" accept=".txt" required>
	<input type="submit" value="Send">
</form>

<form action="/registered-cgi/?xpto=any" method="GET">
	<input type="submit" value="Send">
</form>
```

#### Argument 01, Argument 02, Argument 03, Argument 04 and Argument 05:

Arguments required for the execution of the CGI script/code, or for the program that will execute the CGI script/code. This information is optional. Examples:

```
--silent

--vanilla

--quiet

/home/CGITester/r-CGI/script.R

/home/CGITester/python-CGI/graphic.py
```

### Browser Button

If all the information is correct, pressing this button will open a browser that renders the initial HTML file (if one has been specified); otherwise, the CGI script/code will be called directly.

![](assets/images/i002.png?raw=true "horoscope")

![](assets/images/i003.png?raw=true "map")

