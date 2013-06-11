autoproxy
=========

Autoproxy is a command line tool that automatically detects proxies and stores them in the respective environment variables (e.g. http_proxy).

It sets following environment variables:
* http_proxy
* https_proxy
* ftp_proxy

These environment variables are used by many tool which have a Linux origin (also when used on Windows) - like apt-get GEM, GIT or NPM.

Installation and Usage
----------------------

### On Linux

1. Download: [autoproxy_1.0.tar.gz](http://www.stephan-brenner.com/downloads/autoproxy/autoproxy_1.0.tar.gz) (10 kB)  

2. Installation: Copy the two files from the compressed tar archive to ```/usr/bin/```

3. Usage: Enter ```. autoproxy``` in any directory in your terminal to detect the proxies and set the respective environment variables.  
   Note: The dot is important. Otherwise autoproxy cannot set the environment variables in the calling process.

### On Windows:   

1. Download: [autoproxy_1.0.zip](http://www.stephan-brenner.com/downloads/autoproxy/autoproxy_1.0.zip) (5 kB)  

2. Installation: Copy the two files from the ZIP file to any folder and add this folder to your ```%PATH%``` environment variable.

3. Usage: Enter ```autoproxy``` in your command line window to detect the proxies and set the respective environment variables.

How to build from source code
-----------------------------

1. Download source code:  
```
git clone https://github.com/ymx/autoproxy.git autoproxy
cd autoproxy
```

2. Run CMake:  
```
mkdir bin
cd bin
cmake ..
```

3. Build source code:  
On Linux: ```make```  
On Windows: Open ```autoproxy.sln``` in Visual Studio and build it
