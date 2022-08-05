include(FetchContent)

if(NOT magic_enum_POPULATED)
  FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum
    GIT_TAG        master
  )
  FetchContent_MakeAvailable(magic_enum)
endif()

if(NOT thread_pool_POPULATED)
  FetchContent_Declare(
    thread_pool
    GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
    GIT_TAG        737c6036103ac034682ee11c31634ebe6769781f
  )
  FetchContent_MakeAvailable(thread_pool)
endif()

if(NOT json_POPULATED)
  FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG v3.9.1
  )
  FetchContent_MakeAvailable(json)
endif()

# use trinitycore dep (tswow repo)
if(EXISTS ${CMAKE_SOURCE_DIR}/dep/cxxopts)
  set(cxxopts_include ${CMAKE_SOURCE_DIR}/dep/cxxopts)
# clone it ourselves
elseif(NOT cxxopts_POPULATED)
  FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v3.0.0
  )
  FetchContent_MakeAvailable(cxxopts)
  set(cxxopts_include ${cxxopts_SOURCE_DIR}/include)
endif()

if(NOT TARGET BehaviorTree)
  FetchContent_Declare(
    behaviortree
    GIT_REPOSITORY https://github.com/tswow/behavior-tree.git
    GIT_TAG        master
  )
  FetchContent_MakeAvailable(behaviortree)
endif()

# borrowed function from trinitycore
function(CollectIncludeDirectories current_dir variable)
  list(FIND ARGN "${current_dir}" IS_EXCLUDED)
  if(IS_EXCLUDED EQUAL -1)
    list(APPEND ${variable} ${current_dir})
    file(GLOB SUB_DIRECTORIES ${current_dir}/*)
    foreach(SUB_DIRECTORY ${SUB_DIRECTORIES})
      if(IS_DIRECTORY ${SUB_DIRECTORY})
        CollectIncludeDirectories("${SUB_DIRECTORY}" "${variable}" "${ARGN}")
      endif()
    endforeach()
    set(${variable} ${${variable}} PARENT_SCOPE)
  endif()
endfunction()

CollectIncludeDirectories(${CMAKE_CURRENT_SOURCE_DIR}/src BOT_INCLUDE_DIRS)
file(GLOB_RECURSE BOT_SOURCES_REL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/src *.cpp *.h)
list(FILTER BOT_SOURCES_REL EXCLUDE REGEX "../app/.*" )
set(BOT_SOURCES "")
foreach(source ${BOT_SOURCES_REL})
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${CMAKE_CURRENT_SOURCE_DIR}\\src\\${source}")
    list(APPEND BOT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/${source})
endforeach()

add_library(bots SHARED ${BOT_SOURCES} bots.conf)
target_link_libraries(bots shared trinity-core-interface liblua Detour g3dlib nlohmann_json::nlohmann_json BehaviorTree)

target_include_directories(bots PRIVATE 
  ${magic_enum_SOURCE_DIR}/include
  ${thread_pool_SOURCE_DIR}
  ${cxxopts_include}
  ${sol2_SOURCE_DIR}/include
  ${LUA_ROOT}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Map
  ${BOT_INCLUDE_DIRS}
)

add_custom_command(
  TARGET bots
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
    -DBTSDIR=${behaviortree_SOURCE_DIR}
    -DSDIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DBDIR="${CMAKE_BINARY_DIR}/bin/$<CONFIG>/"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildConfig.cmake
)

add_executable(bots-app app/BotApp.cpp)
add_dependencies(bots-app bots)
target_link_libraries(bots-app bots)
target_include_directories(bots-app PRIVATE
  ${cxxopts_SOURCE_DIR}/include
  ${BOT_INCLUDE_DIRS}
)

set_property(TARGET bots PROPERTY CXX_STANDARD 20)