#!/usr/bin/env python3 -tt

"""This catches const qualifiers for parameters passed by value to functions,
in function declarations. Such qualifiers should only be in the definitions. It
will miss:

* declarations with complex type names (template types) or default value
* assignments, functions with things like __attribute__ and pointers with const
* qualifiers
"""

error_msg = "'const' for a literal parameter is an implementation detail. Remove the const from the interface definition."

regexp = r"""^\s*(?:(static|virtual) +)?(?:const +)?(?:[_a-zA-Z][_a-zA-Z0-9]*::)*[_a-zA-Z][_a-zA-Z0-9]*(?: +const)?(?: *(?:\*(?: *const)?|&))* +[_a-zA-Z][_a-zA-Z0-9]* *\((?:(const +)?(?:[_a-zA-Z][_a-zA-Z0-9]*::)*[_a-zA-Z][_a-zA-Z0-9]*(?: +const)?(?: *(?:\*(?: *const)?|&))*(?: +[_a-zA-Z][_a-zA-Z0-9]*)?, *)*(?:const +(?:[_a-zA-Z][_a-zA-Z0-9]*::)*[_a-zA-Z][_a-zA-Z0-9]*|(?:[_a-zA-Z][_a-zA-Z0-9]*::)*[_a-zA-Z][_a-zA-Z0-9]* +const)(?: +[_a-zA-Z][_a-zA-Z0-9]*)?(?:, *(?:const +)?(?:[_a-zA-Z][_a-zA-Z0-9]*::)*[_a-zA-Z][_a-zA-Z0-9]*(?: +const)?(?: *(?:\*(?: *const)?|&))*)*(?: +[_a-zA-Z][_a-zA-Z0-9]*)?(?:\)(?: *const)?(?: throw *\(.*\))?;|,$)"""

forbidden = [
    '	void truncate(int32_t const count);',
    'void func(const Some_Type,',
    'void func(const Some_Type);',
    'void func(const Some_Type) const;',
    'void func(const Some_Type) const throw (whatever);',
    'void func(const Some_Type abc,',
    'void func(const Some_Type abc);',
    'void func(const Some_Type abc) const;',
    'void func(std::string * const & * foo, const Some_Type,',
    'void func(std::string * const & * foo, const Some_Type);',
    'void func(std::string * const & * foo, const Some_Type) const;',
    'void func(std::string * const & * foo, const Some_Type) const throw (whatever);',
    'void func(std::string * const & * foo, const Some_Type abc,',
    'void func(std::string * const & * foo, const Some_Type abc);',
    'void func(std::string * const & * foo, const Some_Type abc) const;',
    'void func(std::string * const & * ngi, const Some_Type, std::string * const & * foo,',
    'void func(std::string * const & * ngi, const Some_Type, std::string * const & * foo);',
    'void func(std::string * const & * ngi, const Some_Type, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, const Some_Type, std::string * const & * foo) const throw (whatever);',
    'void func(std::string * const & * ngi, const Some_Type abc, std::string * const & * foo,',
    'void func(std::string * const & * ngi, const Some_Type abc, std::string * const & * foo);',
    'void func(std::string * const & * ngi, const Some_Type abc, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, const Some_Type, std::string * const & * foo,',
    'void func(std::string * const & * ngi, const Some_Type, std::string * const & * foo);',
    'void func(std::string * const & * ngi, const Some_Type, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, const Some_Type, std::string * const & * foo) const throw (whatever);',
    'void func(std::string * const & * ngi, const Some_Type abc, std::string * const & * foo,',
    'void func(std::string * const & * ngi, const Some_Type abc, std::string * const & * foo);',
    'void func(std::string * const & * ngi, const Some_Type abc, std::string * const & * foo) const;',
    'void func(Some_Type const,',
    'void func(Some_Type const);',
    'void func(Some_Type const) const;',
    'void func(Some_Type const) const throw (whatever);',
    'void func(Some_Type const abc,',
    'void func(Some_Type const abc);',
    'void func(Some_Type const abc) const;',
    'void func(std::string * const & * foo, Some_Type const,',
    'void func(std::string * const & * foo, Some_Type const);',
    'void func(std::string * const & * foo, Some_Type const) const;',
    'void func(std::string * const & * foo, Some_Type const) const throw (whatever);',
    'void func(std::string * const & * foo, Some_Type const abc,',
    'void func(std::string * const & * foo, Some_Type const abc);',
    'void func(std::string * const & * foo, Some_Type const abc) const;',
    'void func(std::string * const & * ngi, Some_Type const, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type const, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type const, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, Some_Type const, std::string * const & * foo) const throw (whatever);',
    'void func(std::string * const & * ngi, Some_Type const abc, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type const abc, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type const abc, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, Some_Type const, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type const, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type const, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, Some_Type const, std::string * const & * foo) const throw (whatever);',
    'void func(std::string * const & * ngi, Some_Type const abc, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type const abc, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type const abc, std::string * const & * foo) const;',
    ' virtual void func(const Some_Type,',
    ' virtual void * func(const Some_Type,',
    '  virtual std::string & func(const Some_Type,',
]

allowed = [
    'void func(Some_Type,',
    'void func(Some_Type);',
    'void func(Some_Type) const;',
    'void func(Some_Type) const throw (whatever);',
    'void func(Some_Type abc,',
    'void func(Some_Type abc);',
    'void func(Some_Type abc) const;',
    'void func(std::string * const & * foo, Some_Type,',
    'void func(std::string * const & * foo, Some_Type);',
    'void func(std::string * const & * foo, Some_Type) const;',
    'void func(std::string * const & * foo, Some_Type) const throw (whatever);',
    'void func(std::string * const & * foo, Some_Type abc,',
    'void func(std::string * const & * foo, Some_Type abc);',
    'void func(std::string * const & * foo, Some_Type abc) const;',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo) const throw (whatever);',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo) const throw (whatever);',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo) const;',
    'void func(Some_Type,',
    'void func(Some_Type);',
    'void func(Some_Type) const;',
    'void func(Some_Type) const throw (whatever);',
    'void func(Some_Type abc,',
    'void func(Some_Type abc);',
    'void func(Some_Type abc) const;',
    'void func(std::string * const & * foo, Some_Type,',
    'void func(std::string * const & * foo, Some_Type);',
    'void func(std::string * const & * foo, Some_Type) const;',
    'void func(std::string * const & * foo, Some_Type) const throw (whatever);',
    'void func(std::string * const & * foo, Some_Type abc,',
    'void func(std::string * const & * foo, Some_Type abc);',
    'void func(std::string * const & * foo, Some_Type abc) const;',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo) const throw (whatever);',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo) const;',
    'void func(std::string * const & * ngi, Some_Type, std::string * const & * foo) const throw (whatever);',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo,',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo);',
    'void func(std::string * const & * ngi, Some_Type abc, std::string * const & * foo) const;',
    ' virtual void func(Some_Type,',
    ' virtual void * func(Some_Type,',
    '  virtual std::string & func(Some_Type,',
]
