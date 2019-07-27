#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <elf-hook.h>

int global_multiply_cpp(int x, int y)
{
    return x * y;
}

int mock_global_multiply_cpp(int x, int y)
{
    return x + y;
}

static int static_multiply_cpp(int x, int y)
{
    return x * y;
}

int mock_static_multiply_cpp(int x, int y)
{
    return x + y;
}

TEST(ElfHook, Global_CPP_Function)
{
    void *global_multiply_ptr = (void*)global_multiply_cpp;
    void *original = ELF_HOOK(global_multiply_cpp, mock_global_multiply_cpp);
    EXPECT_EQ(original, global_multiply_ptr);
    EXPECT_EQ(global_multiply_cpp(1, 2), 3);
    
    ELF_RESTORE(original);
    EXPECT_EQ(global_multiply_cpp(1, 2), 2);
}

TEST(ElfHook, Static_CPP_Function)
{
    void* original = ELF_HOOK(static_multiply_cpp, mock_static_multiply_cpp);
    EXPECT_EQ(original, nullptr);
    EXPECT_EQ(static_multiply_cpp(1, 2), 2);
}
