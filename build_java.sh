#!/bin/bash

export JAVA_HOME=/home/bureaugau/Projects/Library/graalvm-jdk-21.0.8+12.1

#mvn clean package

${JAVA_HOME}/bin/native-image \
    --shared \
    -cp target/iidm-bridge-java-1.0.0-SNAPSHOT.jar \
    -H:+ReportExceptionStackTraces \
    -march=compatibility \
    --no-fallback \
    --shared  \
    --gc=serial \
    -o libpowsybl-iidm-native \
    --allow-incomplete-classpath \
    --add-exports=org.graalvm.nativeimage.builder/com.oracle.svm.core.c=ALL-UNNAMED
#    -H:CLibraryPath=${PWD}/lib \
#    -H:ReflectionConfigurationFiles=reflection-config.json \
#    -H:ResourceConfigurationFiles=resource-config.json