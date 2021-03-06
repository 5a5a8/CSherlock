# CSherlock

<p align="center">
<img src="./demo.gif"/>
</p>

## Introduction

CSherlock is an implementation of [Sherlock](https://github.com/sherlock-project/sherlock) written in the C programming language.
It is an OSINT tool that allows for checking existence of usernames across various social networks.

This implementation has reduced functionality compared to the original, though appears to
run faster, making it good for basic tests on a large number of users or sites.

Current execution time on my machine is 15-20 seconds for one username checked on 297 sites.
For reference the Python version takes over a minute for a similar test.

Limited testing shows a slightly lower false positive rate than the original implementation.

CSherlock has been tested on ARM64/MacOS and x86_64 linux.

## Description

After starting, CSherlock will parse the arguments and load the website data from its `sites.csv` file.
A csv file of any size can be used as more memory is allocated as needed.
This allows users to replace the built-in csv with their own if they wish.

The array containing each line of the csv file is split between a maximum of 256 threads with a default thread number of 32 (POSIX threads are used for multithreading).
Each thread then parses one of its csv lines at a time and if a regex exists in the data, it will check the username against the regex.
If the username matches the regex, it passes the site data to the `get_request` function to send the appropriate web request.

Note that CSherlock actually checks whether the user _doesn't_ exist on the site.
If it can't find one of the error cases, it concludes that the user exists on the site.
There are four possible error cases - which one we check for depends on the error type stored in the site data within the csv file:
- If the username does not match a 'valid username' regex, the user cannot possibly exist on the site. In this case, we don't need to make a web request at all.
- The website returns a non-success status code. In this case, we usually only need to request the headers rather than the entire page, which speeds things up.
- The website returns an error message. In this case, we need to request the entire page text so that we can search for the error code.
- The website returns some redirect URL. In this case, we only capture the header of the first request and check for a redirect.

libpcre2 is used for regular expression matching, while libcurl is used for making web requests.

## Usage

First install libcurl and libpcre2 if they are not already installed.

On an Ubuntu machine, this would be:
```
sudo apt install libcurl4-openssl-dev
sudo apt install libpcre2-8-0
```

Clone this repository:
```
git clone https://github.com/5a5a8/CSherlock.git
```

Change to the directory and build CSherlock:
```
cd CSherlock
make
```

A `csherlock` executable will be created in the `bin` folder.
Simply run it with:
```
./csherlock --help
```

To get started checking a username, simply use `./csherlock [username]`.
