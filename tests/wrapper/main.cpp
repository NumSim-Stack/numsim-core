#include <gtest/gtest.h>
#include <numsim-core/wrapper.h>

using numsim_core::wrapper;
using numsim_core::cwrapper;

// Test fixture for wrapper tests
class WrapperTest : public ::testing::Test {
protected:
  int value = 42;                 // Mutable int to wrap
  const int constValue = 100;     // Const int to test const wrapper
};

// Test default constructor
TEST_F(WrapperTest, DefaultConstructor) {
  wrapper<int> w; // Default constructor
  EXPECT_FALSE(w); // Should not contain a valid pointer
}

// Test explicit constructor for mutable objects
TEST_F(WrapperTest, WrapsMutableObject) {
  wrapper<int> w(value); // Wrap the mutable object
  EXPECT_TRUE(w);        // Should contain a valid pointer
  EXPECT_EQ(*w, value);  // Dereferencing should return the original value
}

// Test explicit constructor for const objects
TEST_F(WrapperTest, WrapsConstObject) {
  cwrapper<int> w(constValue); // Wrap the const object
  EXPECT_TRUE(w);              // Should contain a valid pointer
  EXPECT_EQ(*w, constValue);   // Dereferencing should return the const value
}

// Test copy constructor
TEST_F(WrapperTest, CopyConstructor) {
  wrapper<int> w1(value); // Wrap the mutable object
  wrapper<int> w2(w1);    // Copy construct from w1
  EXPECT_TRUE(w2);        // w2 should also contain a valid pointer
  EXPECT_EQ(*w2, value);  // Should hold the same value as w1
}

// Test copy assignment operator
TEST_F(WrapperTest, CopyAssignmentOperator) {
  wrapper<int> w1(value); // Wrap the mutable object
  wrapper<int> w2;        // Default constructor
  w2 = w1;                // Copy assign from w1
  EXPECT_TRUE(w2);        // w2 should now contain a valid pointer
  EXPECT_EQ(*w2, value);  // Should hold the same value as w1
}

// Test get() for mutable objects
TEST_F(WrapperTest, GetMutable) {
  wrapper<int> w(value);        // Wrap the mutable object
  EXPECT_EQ(w.get(), value);    // get() should return the original value
  w.get() = 50;                 // Modify the value through the wrapper
  EXPECT_EQ(value, 50);         // Original value should now be modified
}

// Test get() for const objects
TEST_F(WrapperTest, GetConst) {
  cwrapper<int> w(constValue);   // Wrap the const object
  EXPECT_EQ(w.get(), constValue); // get() should return the original value
}

// Test dereference operator
TEST_F(WrapperTest, DereferenceOperator) {
  wrapper<int> w(value); // Wrap the mutable object
  EXPECT_EQ(*w, value);  // Dereferencing should return the original value
}

// Test bool conversion operator
TEST_F(WrapperTest, BoolConversion) {
  wrapper<int> w1;        // Default constructor (null pointer)
  wrapper<int> w2(value); // Wrap a valid object
  EXPECT_FALSE(w1);       // w1 should convert to false
  EXPECT_TRUE(w2);        // w2 should convert to true
}

// Test reassigning a wrapper
TEST_F(WrapperTest, Reassignment) {
  int anotherValue = 123;
  wrapper<int> w1(value);          // Wrap the mutable object
  wrapper<int> w2(anotherValue);   // Wrap a different object
  w2 = w1;                         // Reassign w2 to point to the same object as w1
  EXPECT_EQ(*w2, value);           // w2 should now point to value
}

// Test const-correctness in the wrapper
TEST_F(WrapperTest, ConstCorrectness) {
  const int anotherConstValue = 200;
  cwrapper<int> w(anotherConstValue);  // Wrap a const object
  const int& ref = w.get();            // Use const get()
  EXPECT_EQ(ref, anotherConstValue);   // Should return the correct value
}

// Test the wrapper with nullptr behavior
TEST_F(WrapperTest, NullPointerBehavior) {
  wrapper<int> w;         // Default constructed wrapper (nullptr)
  EXPECT_FALSE(w);        // Should not contain a valid pointer
  //dereferencing is marked noexcept and doesnt throw any exception
  //EXPECT_THROW(*w, std::bad_function_call); // Dereferencing should throw
}

// Test wrapper with a custom object
class TestObject {
public:
  TestObject(int a, double b) : a_(a), b_(b) {}
  int a_;
  double b_;
};

TEST(WrapperCustomObjectTest, WrapCustomObject) {
  TestObject obj(10, 20.5);
  wrapper<TestObject> w(obj); // Wrap a custom object
  EXPECT_TRUE(w);             // Should contain a valid pointer
  EXPECT_EQ(w.get().a_, 10);  // Access members through get()
  EXPECT_EQ(w.get().b_, 20.5);
}

// Test wrapper with const custom object
TEST(WrapperCustomObjectTest, WrapConstCustomObject) {
  const TestObject obj(30, 40.5);
  cwrapper<TestObject> w(obj); // Wrap a const custom object
  EXPECT_TRUE(w);              // Should contain a valid pointer
  EXPECT_EQ(w.get().a_, 30);   // Access members through const get()
  EXPECT_EQ(w.get().b_, 40.5);
}
