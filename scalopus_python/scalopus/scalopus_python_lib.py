
# This function magically makes 'import scalopus_python_lib' load the the shared object
# that's found inside the .egg file instead of 'this' python file.
def __bootstrap__():
    global __bootstrap__, __loader__, __file__
    import sys, pkg_resources, importlib, importlib.machinery, sysconfig
    # Use the ABI suffix, this is "" for python2, for python3 this call returns something like:
    # ['cpython-36m-x86_64-linux-gnu']
    so_abi_suffix = sysconfig.get_config_vars("SOABI")
    so_suffix = ""
    if so_abi_suffix:
        if len(so_abi_suffix):
            so_suffix = so_abi_suffix[0] + "." if so_abi_suffix[0] is not None else ""
    name = __name__
    filename = './scalopus_python_lib.{}so'.format(so_suffix)
    __file__ = pkg_resources.resource_filename(name, filename)
    __loader__ = None; del __bootstrap__, __loader__
    
    import importlib.machinery
    loader = importlib.machinery.ExtensionFileLoader(name, filename)
    spec = importlib.machinery.ModuleSpec(name=name, loader=loader, origin=filename)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module) 
__bootstrap__()
