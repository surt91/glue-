#!/bin/bash

rsync -vur doc/html/{*,mathjax} rasp:portfolio/glue++
