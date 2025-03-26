#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

// Function to compute sum of array
static PyObject* array_sum(PyObject* self, PyObject* args) {
    PyObject* input_list;
    if (!PyArg_ParseTuple(args, "O", &input_list)) {
        return NULL;
    }

    // Convert input to a sequence
    PyObject* seq = PySequence_Fast(input_list, "Argument must be iterable");
    if (!seq) {
        return NULL;
    }

    Py_ssize_t length = PySequence_Fast_GET_SIZE(seq);
    double sum = 0.0;

    for (Py_ssize_t i = 0; i < length; i++) {
        PyObject* item = PySequence_Fast_GET_ITEM(seq, i);
        if (!PyFloat_Check(item) && !PyLong_Check(item)) {
            Py_DECREF(seq);
            PyErr_SetString(PyExc_TypeError, "All elements must be numbers");
            return NULL;
        }
        sum += PyFloat_AsDouble(item);
    }

    Py_DECREF(seq);
    return PyFloat_FromDouble(sum);
}

// Function to compute average
static PyObject* array_average(PyObject* self, PyObject* args) {
    PyObject* input_list;
    if (!PyArg_ParseTuple(args, "O", &input_list)) {
        return NULL;
    }

    PyObject* seq = PySequence_Fast(input_list, "Argument must be iterable");
    if (!seq) {
        return NULL;
    }

    Py_ssize_t length = PySequence_Fast_GET_SIZE(seq);
    if (length == 0) {
        Py_DECREF(seq);
        PyErr_SetString(PyExc_ValueError, "Cannot compute average of empty array");
        return NULL;
    }

    double sum = 0.0;
    for (Py_ssize_t i = 0; i < length; i++) {
        PyObject* item = PySequence_Fast_GET_ITEM(seq, i);
        if (!PyFloat_Check(item) && !PyLong_Check(item)) {
            Py_DECREF(seq);
            PyErr_SetString(PyExc_TypeError, "All elements must be numbers");
            return NULL;
        }
        sum += PyFloat_AsDouble(item);
    }

    Py_DECREF(seq);
    return PyFloat_FromDouble(sum / length);
}

// Function to compute standard deviation
static PyObject* array_std_dev(PyObject* self, PyObject* args) {
    PyObject* input_list;
    if (!PyArg_ParseTuple(args, "O", &input_list)) {
        return NULL;
    }

    PyObject* seq = PySequence_Fast(input_list, "Argument must be iterable");
    if (!seq) {
        return NULL;
    }

    Py_ssize_t length = PySequence_Fast_GET_SIZE(seq);
    if (length <= 1) {
        Py_DECREF(seq);
        PyErr_SetString(PyExc_ValueError, "Std dev requires at least two elements");
        return NULL;
    }

    // First pass: compute mean
    double sum = 0.0;
    double* values = malloc(length * sizeof(double));
    if (!values) {
        Py_DECREF(seq);
        PyErr_SetString(PyExc_MemoryError, "Could not allocate memory");
        return NULL;
    }

    for (Py_ssize_t i = 0; i < length; i++) {
        PyObject* item = PySequence_Fast_GET_ITEM(seq, i);
        if (!PyFloat_Check(item) && !PyLong_Check(item)) {
            free(values);
            Py_DECREF(seq);
            PyErr_SetString(PyExc_TypeError, "All elements must be numbers");
            return NULL;
        }
        values[i] = PyFloat_AsDouble(item);
        sum += values[i];
    }
    double mean = sum / length;

    // Second pass: compute variance
    double variance_sum = 0.0;
    for (Py_ssize_t i = 0; i < length; i++) {
        double diff = values[i] - mean;
        variance_sum += diff * diff;
    }

    free(values);
    Py_DECREF(seq);

    // Compute standard deviation (population std dev)
    return PyFloat_FromDouble(sqrt(variance_sum / length));
}

// Function to compute mode
static PyObject* array_mode(PyObject* self, PyObject* args) {
    PyObject* input_list;
    if (!PyArg_ParseTuple(args, "O", &input_list)) {
        return NULL;
    }

    PyObject* seq = PySequence_Fast(input_list, "Argument must be iterable");
    if (!seq) {
        return NULL;
    }

    Py_ssize_t length = PySequence_Fast_GET_SIZE(seq);
    if (length == 0) {
        Py_DECREF(seq);
        PyErr_SetString(PyExc_ValueError, "Cannot compute mode of empty array");
        return NULL;
    }

    // Safety initialization with error handling
    double mode = 0.0;
    int mode_found = 0;

    // Allocate memory for values and counts
    double* values = malloc(length * sizeof(double));
    int* counts = calloc(length, sizeof(int));
    if (!values || !counts) {
        free(values);
        free(counts);
        Py_DECREF(seq);
        PyErr_SetString(PyExc_MemoryError, "Could not allocate memory");
        return NULL;
    }

    // Convert to array of doubles and count occurrences
    Py_ssize_t unique_count = 0;
    for (Py_ssize_t i = 0; i < length; i++) {
        PyObject* item = PySequence_Fast_GET_ITEM(seq, i);
        if (!PyFloat_Check(item) && !PyLong_Check(item)) {
            free(values);
            free(counts);
            Py_DECREF(seq);
            PyErr_SetString(PyExc_TypeError, "All elements must be numbers");
            return NULL;
        }
        
        double current = PyFloat_AsDouble(item);
        
        // Check if value already exists
        int found = 0;
        for (Py_ssize_t j = 0; j < unique_count; j++) {
            if (fabs(values[j] - current) < DBL_EPSILON) {
                counts[j]++;
                found = 1;
                break;
            }
        }
        
        // If not found, add to unique values
        if (!found) {
            values[unique_count] = current;
            counts[unique_count] = 1;
            unique_count++;
        }
    }

    // Find mode (first most frequent value)
    int max_count = 0;
    for (Py_ssize_t i = 0; i < unique_count; i++) {
        if (counts[i] > max_count) {
            max_count = counts[i];
            mode = values[i];
            mode_found = 1;
        }
    }

    free(values);
    free(counts);
    Py_DECREF(seq);

    // Check if mode was found
    if (!mode_found) {
        PyErr_SetString(PyExc_RuntimeError, "Could not determine mode");
        return NULL;
    }

    return PyFloat_FromDouble(mode);
}

// Function to count array length
static PyObject* array_length(PyObject* self, PyObject* args) {
    PyObject* input_list;
    if (!PyArg_ParseTuple(args, "O", &input_list)) {
        return NULL;
    }

    PyObject* seq = PySequence_Fast(input_list, "Argument must be iterable");
    if (!seq) {
        return NULL;
    }

    Py_ssize_t length = PySequence_Fast_GET_SIZE(seq);
    Py_DECREF(seq);

    return PyLong_FromSsize_t(length);
}

// Method definition object for this extension
static PyMethodDef StatisticalMethods[] = {
    {"array_sum", array_sum, METH_VARARGS, "Compute sum of an array of numbers"},
    {"array_average", array_average, METH_VARARGS, "Compute average of an array of numbers"},
    {"array_std_dev", array_std_dev, METH_VARARGS, "Compute standard deviation of an array of numbers"},
    {"array_mode", array_mode, METH_VARARGS, "Compute mode of an array of numbers"},
    {"array_length", array_length, METH_VARARGS, "Count number of elements in an array"},
    {NULL, NULL, 0, NULL}
};

// Module definition
static struct PyModuleDef statisticalmodule = {
    PyModuleDef_HEAD_INIT,
    "stat_extention",
    NULL,
    -1,
    StatisticalMethods
};

// Module initialization function
PyMODINIT_FUNC PyInit_stat_extention(void) {
    return PyModule_Create(&statisticalmodule);
}