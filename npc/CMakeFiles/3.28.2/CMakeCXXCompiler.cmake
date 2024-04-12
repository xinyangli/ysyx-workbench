set(CMAKE_CXX_COMPILER "/nix/store/0jyc9k3n1vy0dcfci21i1d89kw7v9h76-ccache-links-wrapper-4.9.1/bin/g++")
set(CMAKE_CXX_COMPILER_ARG1 "")
set(CMAKE_CXX_COMPILER_ID "GNU")
set(CMAKE_CXX_COMPILER_VERSION "13.2.0")
set(CMAKE_CXX_COMPILER_VERSION_INTERNAL "")
set(CMAKE_CXX_COMPILER_WRAPPER "")
set(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT "17")
set(CMAKE_CXX_EXTENSIONS_COMPUTED_DEFAULT "ON")
set(CMAKE_CXX_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters;cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates;cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates;cxx_std_17;cxx_std_20;cxx_std_23")
set(CMAKE_CXX98_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters")
set(CMAKE_CXX11_COMPILE_FEATURES "cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates")
set(CMAKE_CXX14_COMPILE_FEATURES "cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates")
set(CMAKE_CXX17_COMPILE_FEATURES "cxx_std_17")
set(CMAKE_CXX20_COMPILE_FEATURES "cxx_std_20")
set(CMAKE_CXX23_COMPILE_FEATURES "cxx_std_23")

set(CMAKE_CXX_PLATFORM_ID "Linux")
set(CMAKE_CXX_SIMULATE_ID "")
set(CMAKE_CXX_COMPILER_FRONTEND_VARIANT "GNU")
set(CMAKE_CXX_SIMULATE_VERSION "")




set(CMAKE_AR "/nix/store/0jyc9k3n1vy0dcfci21i1d89kw7v9h76-ccache-links-wrapper-4.9.1/bin/ar")
set(CMAKE_CXX_COMPILER_AR "/nix/store/34kyxr6f0xa2660j2jr8hs0878sd8ql5-ccache-links-4.9.1/bin/gcc-ar")
set(CMAKE_RANLIB "/nix/store/0jyc9k3n1vy0dcfci21i1d89kw7v9h76-ccache-links-wrapper-4.9.1/bin/ranlib")
set(CMAKE_CXX_COMPILER_RANLIB "/nix/store/34kyxr6f0xa2660j2jr8hs0878sd8ql5-ccache-links-4.9.1/bin/gcc-ranlib")
set(CMAKE_LINKER "/nix/store/0jyc9k3n1vy0dcfci21i1d89kw7v9h76-ccache-links-wrapper-4.9.1/bin/ld")
set(CMAKE_MT "")
set(CMAKE_TAPI "CMAKE_TAPI-NOTFOUND")
set(CMAKE_COMPILER_IS_GNUCXX 1)
set(CMAKE_CXX_COMPILER_LOADED 1)
set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_CXX_ABI_COMPILED TRUE)

set(CMAKE_CXX_COMPILER_ENV_VAR "CXX")

set(CMAKE_CXX_COMPILER_ID_RUN 1)
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS C;M;c++;cc;cpp;cxx;m;mm;mpp;CPP;ixx;cppm;ccm;cxxm;c++m)
set(CMAKE_CXX_IGNORE_EXTENSIONS inl;h;hpp;HPP;H;o;O;obj;OBJ;def;DEF;rc;RC)

foreach (lang C OBJC OBJCXX)
  if (CMAKE_${lang}_COMPILER_ID_RUN)
    foreach(extension IN LISTS CMAKE_${lang}_SOURCE_FILE_EXTENSIONS)
      list(REMOVE_ITEM CMAKE_CXX_SOURCE_FILE_EXTENSIONS ${extension})
    endforeach()
  endif()
endforeach()

set(CMAKE_CXX_LINKER_PREFERENCE 30)
set(CMAKE_CXX_LINKER_PREFERENCE_PROPAGATES 1)
set(CMAKE_CXX_LINKER_DEPFILE_SUPPORTED TRUE)

# Save compiler ABI information.
set(CMAKE_CXX_SIZEOF_DATA_PTR "8")
set(CMAKE_CXX_COMPILER_ABI "ELF")
set(CMAKE_CXX_BYTE_ORDER "LITTLE_ENDIAN")
set(CMAKE_CXX_LIBRARY_ARCHITECTURE "")

if(CMAKE_CXX_SIZEOF_DATA_PTR)
  set(CMAKE_SIZEOF_VOID_P "${CMAKE_CXX_SIZEOF_DATA_PTR}")
endif()

if(CMAKE_CXX_COMPILER_ABI)
  set(CMAKE_INTERNAL_PLATFORM_ABI "${CMAKE_CXX_COMPILER_ABI}")
endif()

if(CMAKE_CXX_LIBRARY_ARCHITECTURE)
  set(CMAKE_LIBRARY_ARCHITECTURE "")
endif()

set(CMAKE_CXX_CL_SHOWINCLUDES_PREFIX "")
if(CMAKE_CXX_CL_SHOWINCLUDES_PREFIX)
  set(CMAKE_CL_SHOWINCLUDES_PREFIX "${CMAKE_CXX_CL_SHOWINCLUDES_PREFIX}")
endif()





set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "/nix/store/bc662545zjk7dkcm5mrzpj111hrgzyq1-openjdk-19.0.2+7/include;/nix/store/gyjvq9wkgz1hmzavf59i4xi0bzs63c0j-gdb-14.1/include;/nix/store/kas3m80ljpr7722q35hi276b0mw81y62-nvboard/include;/nix/store/irqp6shblr91m6mrprjfapw8rz3cpx9a-SDL2-2.30.0-dev/include;/nix/store/hkkwx6m0g0hz87ckjzmyvzxj2gr9fnzn-xorgproto-2023.2/include;/nix/store/477xpc7ffpkds6a20v4gdx9hkq2y08kx-libGL-1.7.0-dev/include;/nix/store/pirdcrf04q6qm2yq8w6w4gam0ax1vlnd-libX11-1.8.7-dev/include;/nix/store/hdzim0zvasrzw9mzh1c4qsqa9d3r5afq-libxcb-1.16-dev/include;/nix/store/yh8p8hbk7d6il917a80kqmidv4cmhczl-SDL2_image-2.8.2/include;/nix/store/nyax32954cfji28r37jaljdr0wfivp95-circt-1.62.0-dev/include;/nix/store/d1g6jdfy1p0bxgl7maxzmcidp19nxba4-tcl-8.6.13/include;/nix/store/zpd0cfr5jnw9ls8i6s9kswbilln25dwn-readline-8.2p10-dev/include;/nix/store/14qma26g4vr24fhszcvpiaqhy5zffyqy-ncurses-6.4-dev/include;/nix/store/7sg7d748qf9lc8cq0ywhs1y2qbxain3n-libffi-3.4.4-dev/include;/nix/store/hwlm6akqm83lapl79y8r9r2vw741sppd-zlib-1.3.1-dev/include;/nix/store/lca036a1dpy1za44y9zdfn401afqzl0v-python3-3.11.8-env/include;/nix/store/d68vcv5yk6f6s0vdf7cmazwy6w1b9mxr-boost-1.81.0-dev/include;/nix/store/gb0ma8iaai115b91fljn0a4f0pc64wyf-cli11-2.3.2/include;/nix/store/d4bigmflznhz06rsrmjs1z0s73bis9vq-flex-2.6.4/include;/nix/store/z5ghkxklksfayrri517mnp67vfj03im5-openssl-3.0.13-dev/include;/nix/store/mz5rqlnmfyq18dvlqkdzi6dm5l9kjpjp-llvm-16.0.6-dev/include;/nix/store/rkja20dpb0nxizalvr6k1phl51mjm6a6-libxml2-2.12.5-dev/include;/nix/store/cmr8qd8w64w8q0cbfc30p98z2pydc1k7-gcc-13.2.0/include/c++/13.2.0;/nix/store/cmr8qd8w64w8q0cbfc30p98z2pydc1k7-gcc-13.2.0/include/c++/13.2.0/x86_64-unknown-linux-gnu;/nix/store/cmr8qd8w64w8q0cbfc30p98z2pydc1k7-gcc-13.2.0/include/c++/13.2.0/backward;/nix/store/cmr8qd8w64w8q0cbfc30p98z2pydc1k7-gcc-13.2.0/lib/gcc/x86_64-unknown-linux-gnu/13.2.0/include;/nix/store/cmr8qd8w64w8q0cbfc30p98z2pydc1k7-gcc-13.2.0/include;/nix/store/cmr8qd8w64w8q0cbfc30p98z2pydc1k7-gcc-13.2.0/lib/gcc/x86_64-unknown-linux-gnu/13.2.0/include-fixed;/nix/store/b0s2lkf593r3585038ws4jd3lylf2wdx-glibc-2.38-44-dev/include")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "stdc++;m;gcc_s;gcc;c;gcc_s;gcc")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "/nix/store/gyjvq9wkgz1hmzavf59i4xi0bzs63c0j-gdb-14.1/lib;/nix/store/kas3m80ljpr7722q35hi276b0mw81y62-nvboard/lib;/nix/store/xav6z6xz0n9bb9zvvvivsjyzyzx4lh9q-libGL-1.7.0/lib;/nix/store/ncwrra4bx8rl39f4bcs4ykav3y98m2lp-libglvnd-1.7.0/lib;/nix/store/klgd6i59s5dgdv9j8ka78s9ggqp8ijnk-libxcb-1.16/lib;/nix/store/zdvnf6b6w5sdgxbzg7v6yrlijml8y4yq-libX11-1.8.7/lib;/nix/store/hq8x2mh8di2ry3m9h9i20n1clzwdlwc3-SDL2-2.30.0/lib;/nix/store/yh8p8hbk7d6il917a80kqmidv4cmhczl-SDL2_image-2.8.2/lib;/nix/store/1cakhvp3yd98qbv7b7xca3b7abxg96d4-circt-1.62.0-lib/lib;/nix/store/d1g6jdfy1p0bxgl7maxzmcidp19nxba4-tcl-8.6.13/lib;/nix/store/5sqn9ipaj6zm26pypl6l748ahw9n6i3f-ncurses-6.4/lib;/nix/store/d48gzzk85sjwxgdrnannajrg3inzmv64-readline-8.2p10/lib;/nix/store/5r3r06j8hzvb2cm0sjhdadgdmzrfh3nj-libffi-3.4.4/lib;/nix/store/bnqa606cwwff6ja8l6gz7milm7ajd1zi-zlib-1.3.1/lib;/nix/store/lca036a1dpy1za44y9zdfn401afqzl0v-python3-3.11.8-env/lib;/nix/store/15c9a9n5pv3ag1sc3r0yfiwzgi4pb9n1-boost-1.81.0/lib;/nix/store/d4bigmflznhz06rsrmjs1z0s73bis9vq-flex-2.6.4/lib;/nix/store/nx0xccx5y1lrmlmi8qx36vqx7l9q9kli-bison-3.8.2/lib;/nix/store/qkfcr92mk15h8hmwzds3g5gkx0vm5l26-openssl-3.0.13/lib;/nix/store/ych6zj36nla5saw00n4cglx1kvxkdii7-llvm-16.0.6-lib/lib;/nix/store/n8dqvi9lp8j8wkfdi0ispkfh2ahhljd1-libxml2-2.12.5/lib;/nix/store/sxr2igfkwhxbagri49b8krmcqz168sim-python3-3.11.8/lib;/nix/store/8mc30d49ghc8m5z96yz39srlhg5s9sjj-glibc-2.38-44/lib;/nix/store/9r8z1qr5jhv90fl2457qqml7qgk88fd6-gcc-13.2.0-lib/lib;/nix/store/0jyc9k3n1vy0dcfci21i1d89kw7v9h76-ccache-links-wrapper-4.9.1/bin;/nix/store/cmr8qd8w64w8q0cbfc30p98z2pydc1k7-gcc-13.2.0/lib/gcc/x86_64-unknown-linux-gnu/13.2.0;/nix/store/cmr8qd8w64w8q0cbfc30p98z2pydc1k7-gcc-13.2.0/lib64;/nix/store/cmr8qd8w64w8q0cbfc30p98z2pydc1k7-gcc-13.2.0/lib")
set(CMAKE_CXX_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES "")
