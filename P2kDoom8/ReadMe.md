# P2kDoom8

## ELF Recipe

## WAD Recipe

```sh
# Optionally.
# Download and install Java JDK 21+ if system JDK version is lower.
export PATH=/jdk-21.0.6/bin:$PATH
export JAVA_HOME=/jdk-21.0.6

# Execute commands from root of jWadUtil project directory.
cd jWadUtil

mvn package -DskipTests=true

java -jar target/jwadutil-1.0-SNAPSHOT.jar
```
