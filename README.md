# CSherlock

## Introduction

CSherlock is an implementation of [Sherlock](https://github.com/sherlock-project/sherlock) written in the C programming language.
It is an OSINT tool that allows for checking existence of usernames across various social networks.

This implementation is still a work in progress and is not ready for public use.
There are some minor bugs as well as a much higher false positive rate than the original implementation of Sherlock.

Note that CSherlock has only been tested on ARM64/MacOS.

## Description

After starting, CSherlock will parse the arguments and load the website data from its `sites.csv` file.
A csv file of any size can be used as more memory is allocated as needed.
This allows users to replace the built-in csv with their own if they wish.

The array containing each line of the csv file is split between a maximum of 32 threads (POSIX threads are used for multithreading).
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

## TODO
- Change 'remainder' sites to be split across existing threads, instead of running in main thread
- Write a proper cross-platform makefile
- Improve false positive rate (i.e. finish implementation of get_request function)
- Complete documentation
- Fix bug where URL is occasionally read as an empty string leading to invalid URL error
- Add licencing and file information to top of files
- Add proper verbose printing for verbose mode
- Improve display of output
