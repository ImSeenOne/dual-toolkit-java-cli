# Project Setup and Execution Guide

## Project structure

   ```
   project-root/
   ├── src/
   │   ├── main/
   │   │   ├── java/
   │   │   ├── resources/
   │   │   └── native/
   ├── pom.xml
   ```

## Instalation instructions
## Linux

**Prerrequisites:**
- C++ compiler (e.g., `g++`)
- Java Development Kit (JDK) 17
- Maven build system

### Step 1: Install Dependencies

Before building Essentia, install the necessary dependencies:

```bash
sudo apt-get update
sudo apt-get install -y build-essential libfftw3-dev libavcodec-dev libavformat-dev libavutil-dev libyaml-dev libtag1-dev
```

### Step 2: Build and Install Essentia

1. **Clone Essentia Repository:**

   ```bash
   git clone https://github.com/MTG/essentia.git
   cd essentia
   ```

2. **Configure with waf:**

   ```bash
   ./waf configure --mode=release --build-static --with-python --with-cpptests --with-examples --with-vamp
   ```

3. **Build Essentia:**

   ```bash
   ./waf
   ```

4. **Install Essentia:**

   ```bash
   sudo ./waf install
   ```

   This installs Essentia into `/usr/local/lib` and `/usr/local/include/essentia`. Please make sure this has happened, depending on the Linux distro you have it may happen or not. If not, just move the files to the needed folders.

### Step 3: Build and Run the Java Application

1. **Build the Project:**
   Compile your Java and native code with Maven:

   ```bash
   mvn clean install
   ```
   
2. **Build classpath:**
   You are gonna have to know where to get the resources when you compile it, to achieve this, run:
   ```bash
   mvn dependency:build-classpath -Dmdep.outputFile=classpath.txt
   ```
   to build your whole classpath for the java project.

3. **Run the Java Application:**
   Execute Dual Toolkit:
   ```bash
   mvn exec:java # TODO: FIX THIS, NOT WORKING PROPERLY
   ```
   OR
   ```bash
   java -Djava.library.path=src/main/resources/lib -cp target/classes:$(cat classpath.txt) -verbose:class com.caveman.dual.DualApplication
   ```
   
