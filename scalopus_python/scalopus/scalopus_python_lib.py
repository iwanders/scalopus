
# This function magically makes 'import scalopus_python_lib' load the the shared object
# that's found inside the .egg file instead of 'this' python file.
def __bootstrap__():
    global __bootstrap__, __loader__, __file__
    import sys, pkg_resources, imp, sysconfig
    # Use the ABI suffix, this is "" for python2, for python3 this call returns something like:
    # ['cpython-36m-x86_64-linux-gnu']
    so_abi_suffix = sysconfig.get_config_vars("SOABI")
    so_suffix = ""
    if so_abi_suffix:
        if len(so_abi_suffix):
            so_suffix = so_abi_suffix[0] + "." if so_abi_suffix[0] is not None else ""
    __file__ = pkg_resources.resource_filename(__name__,'../scalopus_python_lib.{}so'.format(so_suffix))
    __loader__ = None; del __bootstrap__, __loader__
    imp.load_dynamic(__name__,__file__)
__bootstrap__()
