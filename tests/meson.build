test_mac_to_str = executable(
        'test_mac_to_str',
        [meson.project_source_root() / 'server.c', 'test_mac_to_str.c'],
)

test_dhcp_is_invalid_request = executable(
        'test_dhcp_is_invalid_request',
        [meson.project_source_root() / 'server.c', 'test_dhcp_is_invalid_request.c'],
)

test_dhcp_get_request_type = executable(
        'test_dhcp_get_request_type',
        [meson.project_source_root() / 'server.c', 'test_dhcp_get_request_type.c'],
)

get_request_type_options_fuzzer = executable(
        'get_request_type_options_fuzzer',
        [meson.project_source_root() / 'server.c', 'get_request_type_options_fuzzer.c'],
)

test('Test mac_to_str', test_mac_to_str)
test('Test dhcp_is_invalid_request', test_dhcp_is_invalid_request)
test('Test dhcp_get_request_type', test_dhcp_get_request_type)
