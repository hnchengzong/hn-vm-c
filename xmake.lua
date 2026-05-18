set_project("hn-vm-c")
set_version("0.1.3")

set_languages("c11")
set_toolchains("clang")

add_rules("mode.debug", "mode.release")

target("hn-vm-c")
    set_kind("binary")
    add_files("src/**.c")
    add_includedirs("include")
    set_warnings("allextra","error")
    set_targetdir("build/hn_vm_c/$(mode)")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
        add_defines("DEBUG")
    end

    if is_mode("release") then
        set_symbols("hidden")
        set_optimize("fastest")
        add_defines("RELEASE")
        set_strip("all")
        set_warnings("none")
        add_defines("RELEASE")
    end

    target("hn-assembler")
    set_kind("binary")
    add_files("assembler/**.c")
    add_files("src/lib/**.c")
    add_includedirs("include")
    set_warnings("allextra","error")
    set_targetdir("build/assembler/$(mode)")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
        add_defines("DEBUG")
    end

    if is_mode("release") then
        set_symbols("hidden")
        set_optimize("fastest")
        add_defines("RELEASE")
        set_strip("all")
        set_warnings("none")
        add_defines("RELEASE")
    end