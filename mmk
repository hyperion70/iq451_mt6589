#!/bin/bash
Project="wiko"
if [ "$1" = "user" ]
then
	echo ./mk -o=TARGET_BUILD_VARIANT=user $Project $2
	./mk -o=TARGET_BUILD_VARIANT=user $Project $2
else
	echo ./mk $Project $1 $2
	./mk $Project $1 $2
fi