add_rules("mode.debug", "mode.release")

package("rdmapp")
    set_description("The rdmapp package")
    add_deps("asio 1.36.0")
    add_deps("spdlog 1.16.0", {private=true, configs={header_only=true}})
    add_urls("https://github.com/SJTU-DDST/rdmapp.git")
    on_install(function (package)
        local configs = {}
        configs.pic = true
        configs.nortti = false
        configs.examples = false
        import("package.tools.xmake").install(package, configs)
    end)
package_end()


add_requires("rdmapp dev")

target("coverbs-rpc")
    set_kind("binary")
    set_languages("c++20")
    add_packages("rdmapp")
    add_files("src/*.cpp")