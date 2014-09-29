#include <gtest/gtest.h>
#include <arrayfire.h>
#include <af/dim4.hpp>
#include <af/defines.h>
#include <af/traits.hpp>
#include <vector>
#include <iostream>
#include <complex>
#include <string>
#include <testHelpers.hpp>

using std::vector;
using std::string;
using std::cout;
using std::endl;
using af::af_cfloat;
using af::af_cdouble;

template<typename T>
class Grad : public ::testing::Test
{
    public:
        virtual void SetUp() {
            subMat0.push_back({0, 4, 1});
            subMat0.push_back({2, 6, 1});
            subMat0.push_back({0, 2, 1});
        }
        vector<af_seq> subMat0;
};

// create a list of types to be tested
typedef ::testing::Types<float, double, af_cfloat, af_cdouble> TestTypes;

// register the type list
TYPED_TEST_CASE(Grad, TestTypes);

template<typename T>
void gradTest(string pTestFile, const unsigned resultIdx0, const unsigned resultIdx1, bool isSubRef = false, const vector<af_seq> * seqv = nullptr)
{
    vector<af::dim4> numDims;
    vector<vector<T>> in;
    vector<vector<T>> tests;
    readTests<T, T, float>(pTestFile,numDims,in,tests);

    af::dim4 idims = numDims[0];

    af_array inArray = 0;
    af_array tempArray = 0;
    af_array g0Array = 0;
    af_array g1Array = 0;

    if (isSubRef) {
        ASSERT_EQ(AF_SUCCESS, af_create_array(&tempArray, &(in[0].front()), idims.ndims(), idims.get(), (af_dtype) af::dtype_traits<T>::af_type));

        ASSERT_EQ(AF_SUCCESS, af_index(&inArray, tempArray, seqv->size(), &seqv->front()));
    } else {
        ASSERT_EQ(AF_SUCCESS, af_create_array(&inArray, &(in[0].front()), idims.ndims(), idims.get(), (af_dtype) af::dtype_traits<T>::af_type));
    }

    ASSERT_EQ(AF_SUCCESS, af_gradient(&g0Array, &g1Array, inArray));

    size_t nElems = tests[resultIdx0].size();
    // Get result
    T* grad0Data = new T[tests[resultIdx0].size()];
    ASSERT_EQ(AF_SUCCESS, af_get_data_ptr((void*)grad0Data, g0Array));

    // Compare result
    for (size_t elIter = 0; elIter < nElems; ++elIter) {
        ASSERT_EQ(tests[resultIdx0][elIter], grad0Data[elIter]) << "at: " << elIter << std::endl;
    }

    // Get result
    T* grad1Data = new T[tests[resultIdx1].size()];
    ASSERT_EQ(AF_SUCCESS, af_get_data_ptr((void*)grad1Data, g1Array));

    // Compare result
    for (size_t elIter = 0; elIter < nElems; ++elIter) {
        ASSERT_EQ(tests[resultIdx1][elIter], grad1Data[elIter]) << "at: " << elIter << std::endl;
    }


    // Delete
    delete[] grad0Data;
    delete[] grad1Data;

    if(inArray   != 0) af_destroy_array(inArray);
    if(g0Array   != 0) af_destroy_array(g0Array);
    if(g1Array   != 0) af_destroy_array(g1Array);
    if(tempArray != 0) af_destroy_array(tempArray);
}

#define GRAD_INIT(desc, file, resultIdx0, resultIdx1)                                       \
    TYPED_TEST(Grad, desc)                                                                  \
    {                                                                                       \
        gradTest<TypeParam>(string(TEST_DIR"/grad/"#file".test"), resultIdx0, resultIdx1);  \
    }

    GRAD_INIT(Grad0, grad, 0, 1);
    GRAD_INIT(Grad1, grad2D, 0, 1);
    GRAD_INIT(Grad2, grad3D, 0, 1);