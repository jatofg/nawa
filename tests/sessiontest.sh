#!/bin/bash
# test script for concurrency with session variables
# replace the URL by a URL that calls the running libsessiontest.so
# and the session ID by one that really exists (create a session by 
# calling the URL in your browser first and use the developer tools 
# to receive the cookie)
# then switch to an empty test dir, run this script here, and have a 
# look at the newly created results[1-100].txt files afterwards
for i in {1..100}
do
	curl -b "SESSION=44fdbfb68ed146c7448b3519cd806eab55dc8ca0" http://t1.local/test?it=[1-100] > "results$i.txt" &
done
wait
