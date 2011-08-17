#define PY_SSIZE_T_CLEAN 1

#include <Python.h>
#include <osdefs.h>
#include <structmember.h>

#include <libtextcat/textcat.h>
#include <libtextcat/constants.h>
#ifdef _UTF8_
#include <libtextcat/utf8misc.h>
#else
#define utfstrlen(s) strlen(s)
#endif

typedef struct
{
        PyObject_HEAD
        void *handle;
} libTextCatObject;

static PyObject	*shortException = NULL,
		*unknownException = NULL;

static const char __author__[] =
"The textcat python module was written by:\n\
\n\
    Per Øyvind Karlsen <peroyvind@mandriva.org>\n\
\n\
Ported to Python 3 by:\n\
	Brendan Molloy <brendan@bbqsrc.net>\n\
";

#ifdef _UTF8_
PyDoc_STRVAR(libTextCat_init__doc__,
"TextCat(conffile, [prefix]) -- Initialize the text classifier.\n\
The textfile conffile should contain a list\n\
of fingerprint filenames and identification strings for the categories.\n\
The filenames should be reachable from either the current working\n\
directory, or the directory specified by 'prefix' argument if given.\n\
The identification strings will be used in the classification output.");
#else
PyDoc_STRVAR(libTextCat_init__doc__,
"TextCat(conffile) -- Initialize the text classifier.\n\
The textfile conffile should contain a list\n\
of fingerprint filenames and identification strings for the categories.\n\
The filenames should be reachable from the current working directory.\n\
The identification strings will be used in the classification output.");
#endif
static int
libTextCat_init(libTextCatObject *self, PyObject *args)
{
	const char *confFile;
#ifdef _UTF8_
	char prefix[MAXPATHLEN];
	const char *path = NULL;

	if (!PyArg_ParseTuple(args, "s|s:init",
			  &confFile, &path))
#else
	if (!PyArg_ParseTuple(args, "s:init",
			  &confFile))
#endif
		return -1;

#ifdef _UTF8_
	if(path != NULL) {
		PyOS_snprintf(prefix, sizeof(prefix), "%s%c", path, SEP);
		self->handle = special_textcat_Init(confFile, prefix);
	} else
#endif
		self->handle = textcat_Init(confFile);
	if(self->handle == NULL){
		PyErr_SetString(PyExc_Exception, "cannot read config file or fingerprint files listed in it.");
		return -1;
	}

	return 0;

}

PyDoc_STRVAR(libTextCat_classify__doc__,
"classify(buffer, [size]) -- Return the most likely categories for buffer.\n\
By default the full buffer will be processed, but the length of buffer to\n\
process can be specified by the optional 'size' argument");
static PyObject*
libTextCat_classify(libTextCatObject *self, PyObject *args)
{
	Py_ssize_t size = 0;
	char *buffer = NULL;
	char *classified, *language;
	PyObject *languages = NULL;
#ifdef _UTF8_
	char *ascii = NULL;
	PyObject *input = NULL,
		 *unicode = NULL,
		 *rawUnicode = NULL;

	if(!PyArg_ParseTuple(args, "O|i", &input, &size))
		return NULL;

	if(PyUnicode_Check(input)) {
	    rawUnicode = PyUnicode_AsRawUnicodeEscapeString(input);
	    buffer = PyString_AsString(rawUnicode);
	    if(!size)
		size = utfstrlen(buffer);
	} else if(PyString_Check(input)) {
	    ascii = PyString_AsString(input);
	    unicode = PyUnicode_FromString(ascii);
	    rawUnicode = PyUnicode_AsRawUnicodeEscapeString(unicode);
	    buffer = PyString_AsString(rawUnicode);
	    if(!size)
		size = utfstrlen(buffer);
	} else {
	    PyErr_SetString(PyExc_TypeError, "Buffer needs to be either of type string/bytes or unicode");
	    goto error;
	}
#else
       if(!PyArg_ParseTuple(args, "s#|i", &buffer, &size, &size))
	       return NULL;
#endif

	if((Py_ssize_t)utfstrlen(buffer) < size){
		PyErr_WarnEx(PyExc_Warning,
				"specified buffer size is larger than the "
				"actual buffer, processing the full buffer in "
				"stead...", 1);
		PyErr_Print();
		size = utfstrlen(buffer);
	}

	classified = textcat_Classify(self->handle, buffer, size);
	if(!strcmp(classified, _TEXTCAT_RESULT_SHORT)){
		PyErr_SetString(shortException, "Document was too short to make a reliable assessment.");
		goto error;
	}

	if(!strcmp(classified, _TEXTCAT_RESULT_UNKOWN)){
		PyErr_SetString(unknownException, "Unable to recognize any languages from document.");
		goto error;
	}

	languages = PyList_New(0);
	while((language = strtok_r(classified, "[", &classified)) != NULL){
		PyObject *lang = PyUnicode_FromStringAndSize(language, strlen(language)-1);
		Py_INCREF(lang);
		if(PyList_Append(languages, lang) < 0)
			goto error;
	}
	Py_INCREF(languages);
	goto end;

 error:
	Py_XDECREF(languages);
 end:
#ifdef _UTF8_
	Py_XDECREF(rawUnicode);
	Py_XDECREF(unicode);
	Py_XDECREF(ascii);
	Py_XDECREF(buffer);
#endif

	return languages;

}

static PyMethodDef libTextCatObject_methods[] = {
    	{"classify", (PyCFunction)libTextCat_classify, METH_VARARGS | METH_KEYWORDS,
		libTextCat_classify__doc__},
	
	{0, 0, 0, 0}
};

static PyObject *
libTextCatObject_new(PyTypeObject *type, __attribute__((unused)) PyObject *args, __attribute__((unused)) PyObject *kwargs)
{
    libTextCatObject *self;
    self = (libTextCatObject *)type->tp_alloc(type, 0);
    if(self == NULL){
	    Py_DECREF(self);
	    return NULL;
    }

    return (PyObject *)self;
}

static void
libTextCat_dealloc(libTextCatObject *self)
{
	if(self->handle != NULL)
		textcat_Done(self->handle);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyTypeObject
libTextCat_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)		/*ob_size*/
	"textcat.TextCat",				/*tp_name*/
	sizeof(libTextCatObject),			/*tp_basicsize*/
	0,						/*tp_itemsize*/
	(destructor)libTextCat_dealloc,			/*tp_dealloc*/
	0,						/*tp_print*/
	0,						/*tp_getattr*/
	0,						/*tp_setattr*/
	0,						/*tp_compare*/
	0,						/*tp_repr*/
	0,						/*tp_as_number*/
	0,						/*tp_as_sequence*/
	0,						/*tp_as_mapping*/
	0,						/*tp_hash*/
	0,						/*tp_call*/
	0,						/*tp_str*/
	0,						/*tp_getattro*/
	0,						/*tp_setattro*/
	0,						/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,		/*tp_flags*/
	libTextCat_init__doc__,         		/*tp_doc*/
	0,						/*tp_traverse*/
	0,						/*tp_clear*/
	0,						/*tp_richcompare*/
	0,						/*tp_weaklistoffset*/
	0,						/*tp_iter*/
	0,						/*tp_iternext*/
	libTextCatObject_methods,			/*tp_methods*/
	0,						/*tp_members*/
	0,						/*tp_getset*/
	0,						/*tp_base*/
	0,						/*tp_dict*/
	0,						/*tp_descr_get*/
	0,						/*tp_descr_set*/
	0,						/*tp_dictoffset*/
	(initproc)libTextCat_init,			/*tp_init*/
	PyType_GenericAlloc,				/*tp_alloc*/
	libTextCatObject_new,				/*tp_new*/
	0,						/*tp_free*/
	0,						/*tp_is_gc*/
	0,						/*tp_bases*/
	0,						/*tp_mro*/
	0,						/*tp_cache*/
	0,						/*tp_subclasses*/
	0,						/*tp_weaklist*/
	0						/*tp_del*/
};

static PyMethodDef
textcat_methods[] = {
	{0, 0, 0, 0}
};

PyDoc_STRVAR(textcat_module_documentation,
"The python libTextCat module provides a interface for the libTextCat\n\
library.");

/* declare function before defining it to avoid compile warnings */
PyMODINIT_FUNC
PyInit_textcat(void)
{
    PyObject *m;
    if (PyType_Ready(&libTextCat_Type) < 0)
 	    return;
    //m = Py_InitModule3("textcat", textcat_methods,
	//	       textcat_module_documentation);
	static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "textcat",     /* m_name */
        textcat_module_documentation,  /* m_doc */
        -1,                  /* m_size */
        textcat_methods,    /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };

	m = PyModule_Create(&moduledef);
    if (m == NULL)
		return NULL;
    shortException = PyErr_NewException("TextCat.ShortException", NULL, NULL);
    if (shortException != NULL) {
        Py_INCREF(shortException);
		PyModule_AddObject(m, "ShortException", shortException);
    }

    unknownException = PyErr_NewException("TextCat.UnknownException", NULL, NULL);
    if (unknownException != NULL) {
        Py_INCREF(unknownException);
		PyModule_AddObject(m, "UnknownException", unknownException);
    }

    Py_INCREF(&libTextCat_Type);
    PyModule_AddObject(m, "TextCat", (PyObject *)&libTextCat_Type);

    PyModule_AddObject(m, "__author__", PyBytes_FromString(__author__));

    PyModule_AddStringConstant(m, "__version__", VERSION);
    PyModule_AddStringConstant(m, "TextCat_version", textcat_Version());
	return m;
}

