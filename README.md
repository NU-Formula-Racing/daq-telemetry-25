# daq-telemetry-25

This repository is the template for DAQ projects. The base is configured to work with ESP32-based projects, which may not be the case for your project. Below are some resources!

This project follows the follows a slightly modified PlatormIO project structure:
```
project-name-25/
|-- include/
|   |-- any .hpp files that are specific to your implementation
|-- src/
|   |-- any .cpp files that are specific to your implementation
|   |-- main.cpp -- this is the entry point for your project
|-- lib/
|   |-- any libraries that are specific to your implementation (that you couldn't include via platformio.ini)
|-- test/
|   |-- any test files that you write for your implementation
|-- .gitignore
|-- platformio.ini -- this is the configuration file for your project
|-- README.md
```


## General Resources
* [Arduino Framework Documentation](https://www.arduino.cc/reference/en/)
* [How to use Git and Github](https://product.hubspot.com/blog/git-and-github-tutorial-for-beginners)
* [How to write Good Pull Requests](https://developers.google.com/blockly/guides/contribute/get-started/write_a_good_pr)
* [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)

## Formula Specific
* [How to Write a Driver! Example](https://github.com/NU-Formula-Racing/daq-driver-example-25): Please look through this one! We need you to write a driver for your project, as your firmware will ultimately be used as a library.
* [Non-Trivial Driver/Library Example (daqser)](https://github.com/NU-Formula-Racing/daq-serializer-24): This is a more complex example of a driver. It is a library that we used in NFR24 to serialize data for telemetry.
* [CAN Example](https://github.com/NU-Formula-Racing/CAN_Interface_Demo/blob/main/src/main.cpp): This is an example, written by the creator of our CAN Library detailing all the common use cases for the library.
* [Project Example (temp-board-23)](https://github.com/NU-Formula-Racing/daq-firmware-23/blob/jm/daqTemp/src/main.cpp): This is an example of an older project, and it might be helpful to see how we structured our code in the past.

## Important Notes
* In formula, we use a branching naming convention ```<initials>/<branch-name>```. So if I were to make a branch called ```analysis-backend```, it should be called ```ebs/analysis-backend```. Notice how we use kebab case, which is the standard in git related things.