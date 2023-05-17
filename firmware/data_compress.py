#!/bin/python
Import("env")
print(env)
# Build actions
print("--------------------BEFORE BUILD-------------------")
print("Running gulp")
env.Execute("gulp")
