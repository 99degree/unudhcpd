test_mac_to_str = executable(
        'test_mac_to_str',
        [meson.source_root() / 'server.c', 'test_mac_to_str.c'],
)

test_dhcp_is_invalid_request = executable(
        'test_dhcp_is_invalid_request',
        [meson.source_root() / 'server.c', 'test_dhcp_is_invalid_request.c'],
)

test_dhcp_get_request_type = executable(
        'test_dhcp_get_request_type',
        [meson.source_root() / 'server.c', 'test_dhcp_get_request_type.c'],
)


test('Test mac_to_str', test_mac_to_str)
test('Test dhcp_is_invalid_request', test_dhcp_is_invalid_request)
test('Test dhcp_get_request_type', test_dhcp_get_request_type)
