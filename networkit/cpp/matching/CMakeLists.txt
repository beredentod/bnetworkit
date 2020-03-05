networkit_add_module(matching
    DynamicSuitorMatcher.cpp
    SuitorMatcher.cpp
    LocalMaxMatcher.cpp
    Matcher.cpp
    Matching.cpp
    PathGrowingMatcher.cpp
    )

networkit_module_link_modules(matching
    auxiliary base graph structures)

add_subdirectory(test)

