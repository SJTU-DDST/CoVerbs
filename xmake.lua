add_rules("mode.debug", "mode.check", "mode.release")

package("rdmapp")
    set_description("The rdmapp package")
    add_deps("asio 1.36.0")
    add_deps("spdlog 1.16.0", {private=true, configs={header_only=true}})
    add_urls("https://github.com/SJTU-DDST/rdmapp.git")
    on_install(function (package)
        local configs = {}
        if package:config("shared") then
            configs.kind = "shared"
        end
        configs.pic = true
        configs.nortti = false
        configs.examples = false
        import("package.tools.xmake").install(package, configs)
    end)
package("cppcoro")
    set_urls("https://github.com/andreasbuhr/cppcoro.git")
    on_install("linux", function(package)
        local configs = {}
        import("package.tools.cmake").install(package, configs)
    end)
package_end()

set_languages("c++20", { public = true })
set_warnings("all", "extra", "pedantic", "error", {private=true})

add_requires("rdmapp dev")
add_requires("cppcoro main")
add_requires("spdlog 1.16.0", {private=true, configs={header_only=true}})

add_includedirs("include")


target("coverbs-rpc")
    set_kind("static")
    add_packages("rdmapp", "cppcoro", "spdlog" , {public=true})
    add_files("src/conn/*.cc")

target("naive_test")
    set_kind("binary")
    add_files("src/tests/naive.cc")
    add_deps("coverbs-rpc")