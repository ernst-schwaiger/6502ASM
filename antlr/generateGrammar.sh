#!/bin/sh
#JAVA_PATH=/cygdrive/c/Program Files/Java/jre1.8.0_161/bin/java
export ANTLR_PATH=/home/ernst/projects/My6502/antlr-4.13.0-complete.jar

java -cp "$ANTLR_PATH:$CLASSPATH" org.antlr.v4.Tool ./MOS6502.g4 -o ./src/generated/antlr4

