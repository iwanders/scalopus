try:
    import scalopus_python_lib
except ModuleNotFoundError as e:
    print("{}\nWas the shared object in your PYTHONPATH variable?".format(str(e)))
