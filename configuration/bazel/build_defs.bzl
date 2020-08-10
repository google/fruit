load("@rules_cc//cc:action_names.bzl", "C_COMPILE_ACTION_NAME")
load("@rules_cc//cc:toolchain_utils.bzl", "find_cpp_toolchain")

def _generate_fruit_config_impl(ctx):
    cc_toolchain = find_cpp_toolchain(ctx)

    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
        requested_features = ctx.features,
        unsupported_features = ctx.disabled_features,
    )
    c_compiler_path = cc_common.get_tool_for_action(
        feature_configuration = feature_configuration,
        action_name = C_COMPILE_ACTION_NAME,
    )

    check_output_files = []
    for check_source in ctx.files.check_sources:
        check_name = check_source.path[:-len(".cpp")].split('/')[-1].split('\\')[-1]

        output_file = ctx.actions.declare_file(check_name + ".o")

        c_compile_variables = cc_common.create_compile_variables(
            feature_configuration = feature_configuration,
            cc_toolchain = cc_toolchain,
            user_compile_flags = ctx.fragments.cpp.copts + ctx.fragments.cpp.conlyopts,
            source_file = check_source.path,
            output_file = output_file.path,
        )
        command_line = cc_common.get_memory_inefficient_command_line(
            feature_configuration = feature_configuration,
            action_name = C_COMPILE_ACTION_NAME,
            variables = c_compile_variables,
        )
        env = cc_common.get_environment_variables(
            feature_configuration = feature_configuration,
            action_name = C_COMPILE_ACTION_NAME,
            variables = c_compile_variables,
        )

        check_define = 'FRUIT_HAS_%s' % check_name.upper()
        check_output_file = ctx.actions.declare_file(check_name + ".h")

        ctx.actions.run_shell(
            command = '"$@" &>/dev/null && echo "#define %s 1" >"%s" || echo "#define %s 0" >"%s"; touch "%s"' % (
                check_define, check_output_file.path, check_define, check_output_file.path, output_file.path
            ),
            arguments = [c_compiler_path] + command_line,
            env = env,
            inputs = depset(
                [check_source],
                transitive = [cc_toolchain.all_files],
            ),
            outputs = [output_file, check_output_file],
        )
        check_output_files.append(check_output_file)

    merged_output_file = ctx.actions.declare_file("fruit/impl/fruit-config-base.h")
    ctx.actions.run_shell(
        command = '\n'.join([
            '(',
            'echo "#ifndef FRUIT_CONFIG_BASE_H"',
            'echo "#define FRUIT_CONFIG_BASE_H"',
            'echo "#define FRUIT_USES_BOOST 1"',
            'cat %s' % ' '.join([check_output_file.path for check_output_file in check_output_files]),
            'echo "#endif"',
            ')>%s' % merged_output_file.path
        ]),
        inputs = check_output_files,
        outputs = [merged_output_file],
    )

    compilation_context, compilation_outputs = cc_common.compile(
        actions = ctx.actions,
        feature_configuration = feature_configuration,
        cc_toolchain = cc_toolchain,
        public_hdrs = [merged_output_file],
        name = "%s_link" % ctx.label.name,
    )

    linking_context, linking_outputs = cc_common.create_linking_context_from_compilation_outputs(
        actions = ctx.actions,
        feature_configuration = feature_configuration,
        compilation_outputs = compilation_outputs,
        cc_toolchain = cc_toolchain,
        name = "%s_link" % ctx.label.name,
    )

    return [
        DefaultInfo(files = depset([merged_output_file]), runfiles = ctx.runfiles(files = [merged_output_file])),
        CcInfo(compilation_context=compilation_context, linking_context=linking_context),
    ]

generate_fruit_config = rule(
    implementation = _generate_fruit_config_impl,
    attrs = {
        "check_sources": attr.label_list(allow_files = True),
        "_cc_toolchain": attr.label(default = Label("@bazel_tools//tools/cpp:current_cc_toolchain")),
    },
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
    fragments = ["cpp"],
)