#include <cassert>

#define DISALLOW_COPY_AND_ASSIGN(ClassName) \
  ClassName(const ClassName&) = delete;     \
  void operator=(const ClassName&) = delete;

#define DISALLOW_CONSTRUCTORS_AND_DESTRUCTORS(ClassName) \
  ClassName() = delete;                                  \
  ~ClassName() = delete;

#define INSTANCE_CLASS(ClassName)   \
 public:                            \
  static ClassName& GetInstance() { \
    static ClassName instance;      \
    return instance;                \
  }                                 \
                                    \
 private:                           \
  ClassName() = default;

#define ASSERT_NULLPTR(...)                \
  do {                                     \
    __assert_nullptr_impl_(##__VA_ARGS__); \
  } while (0)

#define DELETE_ARRAY(...)                \
  do {                                   \
    __delete_array_impl_(##__VA_ARGS__); \
  } while (0)

#define MALLOC_ARRAY(len, ...)                \
  do {                                        \
    assert(len > 0);                          \
    __malloc_array_impl_(len, ##__VA_ARGS__); \
  } while (0)

template <typename T>
inline void __assert_nullptr_impl_(T&& arg) {
  assert(std::forward<T>(arg) != nullptr);
}

template <typename T, typename... Args>
inline void __assert_nullptr_impl_(T&& arg, Args&&... args) {
  __assert_nullptr_impl_(std::forward<T>(arg));
  __assert_nullptr_impl_(std::forward<Args>(args)...);
}

template <typename T>
inline void __delete_array_impl_(T*& arg) {
  if (arg != nullptr) {
    delete[] arg;
    arg = nullptr;
  }
}

template <typename T, typename... Args>
inline void __delete_array_impl_(T*& arg, Args&&... args) {
  __delete_array_impl_(arg);
  __delete_array_impl_(std::forward<Args>(args)...);
}

template <typename T>
inline void __malloc_array_impl_(size_t len, T*& arg) {
  arg = new T[len];
}

template <typename T, typename... Args>
inline void __malloc_array_impl_(size_t len, T*& arg, Args&&... args) {
  __malloc_array_impl_(len, arg);
  __malloc_array_impl_(len, std::forward<Args>(args)...);
}
