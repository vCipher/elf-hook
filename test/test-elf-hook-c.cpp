#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <elf-hook.h>

extern "C"
{
    int global_multiply(int x, int y)
    {
        return x * y;
    }

    int mock_global_multiply(int x, int y)
    {
        return x + y;
    }

    static int static_multiply(int x, int y)
    {
        return x * y;
    }

    static int mock_static_multiply(int x, int y)
    {
        return x + y;
    }
}

TEST(ElfHook, Global_C_Function)
{
    void *global_multiply_ptr = (void*)global_multiply;
    void *original = ELF_HOOK(global_multiply, mock_global_multiply);
    EXPECT_EQ(original, global_multiply_ptr);
    EXPECT_EQ(global_multiply(1, 2), 3);
    
    ELF_RESTORE(original);
    EXPECT_EQ(global_multiply(1, 2), 2);
}

TEST(ElfHook, Static_C_Function)
{
    void* original = ELF_HOOK(static_multiply, mock_static_multiply);
    EXPECT_EQ(original, nullptr);
    EXPECT_EQ(static_multiply(1, 2), 2);
}
