#include <stdio.h>
#include <string.h>
#include <assert.h> // For assert()

#include "xml_utils.h" // For XML helper function declarations and error codes

static int test_find_tag_value_start() {
    printf("Running tests for find_tag_value_start...\n");
    int tests_passed = 0;
    int tests_run = 0;
    const char* result_ptr;

    const char* xml1 = "<test>value</test>";
    tests_run++;
    result_ptr = find_tag_value_start(xml1, "test");
    if (result_ptr != NULL && strcmp(result_ptr, "value</test>") == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC1\n"); assert(0); }

    const char* xml2 = "<outer><test>val</test></outer>";
    tests_run++;
    result_ptr = find_tag_value_start(xml2, "test");
    if (result_ptr != NULL && strcmp(result_ptr, "val</test></outer>") == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC2\n"); assert(0); }

    const char* xml3 = "<test></test>";
    tests_run++;
    result_ptr = find_tag_value_start(xml3, "test");
    if (result_ptr != NULL && strcmp(result_ptr, "</test>") == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC3\n"); assert(0); }

    const char* xml4 = "<anothertag>value</anothertag><test>value2</test>";
    tests_run++;
    result_ptr = find_tag_value_start(xml4, "test");
    if (result_ptr != NULL && strcmp(result_ptr, "value2</test>") == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC4\n"); assert(0); }

    tests_run++;
    result_ptr = find_tag_value_start(xml1, "nonexistent"); // Expected: find_tag_value_start returns NULL
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC5 (nonexistent)\n"); assert(0); }

    tests_run++;
    result_ptr = find_tag_value_start(NULL, "test"); // Expected: find_tag_value_start returns NULL (bad param)
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC6 (NULL xml)\n"); assert(0); }

    tests_run++;
    result_ptr = find_tag_value_start(xml1, NULL); // Expected: find_tag_value_start returns NULL (bad param)
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC7 (NULL tag)\n"); assert(0); }

    // Test for snprintf failure in find_tag_value_start (e.g. excessively long tag name if not handled)
    // This is implicitly tested by get_tag_string's XML_UTIL_ERROR_TAG_NOT_FOUND case if find_tag_value_start returns NULL.
    // No direct test here as find_tag_value_start itself doesn't return specific error codes.

    printf("find_tag_value_start tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}

static int test_get_tag_string() {
    printf("Running tests for get_tag_string...\n");
    char buffer[50];
    int tests_passed = 0;
    int tests_run = 0;
    int res;

    // Test case 1: Simple extraction (value fits)
    const char* xml1 = "<test>value</test>";
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml1, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_SUCCESS && strcmp(buffer, "value") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string TC1 - Expected ret XML_UTIL_SUCCESS (%d), buf 'value'. Got %d, '%s'\n", XML_UTIL_SUCCESS, res, buffer); assert(0); }

    // Test case 2: Nested tag extraction (value fits)
    const char* xml2 = "<outer><test>val</test></outer>";
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml2, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_SUCCESS && strcmp(buffer, "val") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string TC2 - Expected ret XML_UTIL_SUCCESS (%d), buf 'val'. Got %d, '%s'\n", XML_UTIL_SUCCESS, res, buffer); assert(0); }

    // Test case 3: Empty value tag
    const char* xml3 = "<test></test>";
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml3, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_SUCCESS && strcmp(buffer, "") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string TC3 - Expected ret XML_UTIL_SUCCESS (%d), buf ''. Got %d, '%s'\n", XML_UTIL_SUCCESS, res, buffer); assert(0); }

    // Test case 4: Value too long for buffer
    const char* xml_long_val = "<item>verylongvalue</item>"; // length 15
    char small_buf[10]; // Can hold 9 chars + null
    tests_run++;
    small_buf[0] = 'X';
    res = get_tag_string(xml_long_val, "item", small_buf, sizeof(small_buf));
    if (res == XML_UTIL_ERROR_BUFFER_TOO_SMALL && small_buf[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC4 - Expected ret XML_UTIL_ERROR_BUFFER_TOO_SMALL (%d), empty buf. Got %d, '%s'\n", XML_UTIL_ERROR_BUFFER_TOO_SMALL, res, small_buf); assert(0); }

    // Test case 5: Value fits exactly in buffer
    const char* xml_exact_val = "<item>exactfit</item>"; // length 8
    char exact_buf[9];
    tests_run++;
    exact_buf[0] = 'X';
    res = get_tag_string(xml_exact_val, "item", exact_buf, sizeof(exact_buf));
    if (res == XML_UTIL_SUCCESS && strcmp(exact_buf, "exactfit") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string TC5 - Expected ret XML_UTIL_SUCCESS (%d), buf 'exactfit'. Got %d, '%s'\n", XML_UTIL_SUCCESS, res, exact_buf); assert(0); }

    // Test case 6: Value just one char too long for buffer (value len == buffer size)
    const char* xml_just_too_long = "<item>toolonggg</item>"; // length 9
    char just_too_long_buf[9]; // Can hold 8 chars + null
    tests_run++;
    just_too_long_buf[0] = 'X';
    res = get_tag_string(xml_just_too_long, "item", just_too_long_buf, sizeof(just_too_long_buf));
    if (res == XML_UTIL_ERROR_BUFFER_TOO_SMALL && just_too_long_buf[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC6 - Expected ret XML_UTIL_ERROR_BUFFER_TOO_SMALL (%d), empty buf. Got %d, '%s'\n", XML_UTIL_ERROR_BUFFER_TOO_SMALL, res, just_too_long_buf); assert(0); }

    // Test case 7: Non-existent tag
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml1, "nonexistent", buffer, sizeof(buffer));
    if (res == XML_UTIL_ERROR_TAG_NOT_FOUND && buffer[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC7 - Expected ret XML_UTIL_ERROR_TAG_NOT_FOUND (%d), empty buf. Got %d, '%s'\n", XML_UTIL_ERROR_TAG_NOT_FOUND, res, buffer); assert(0); }

    // Test case 8: Tag exists but no closing tag
    const char* xml_no_close = "<test>value_no_close";
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml_no_close, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_ERROR_MALFORMED_XML && buffer[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC8 - Expected ret XML_UTIL_ERROR_MALFORMED_XML (%d), empty buf. Got %d, '%s'\n", XML_UTIL_ERROR_MALFORMED_XML, res, buffer); assert(0); }

    // Test case 9: Buffer size 1 (value "a" is too large)
    const char* xml_val_a = "<test>a</test>";
    char tiny_buffer_get[1];
    tests_run++;
    tiny_buffer_get[0] = 'X';
    res = get_tag_string(xml_val_a, "test", tiny_buffer_get, sizeof(tiny_buffer_get));
    if (res == XML_UTIL_ERROR_BUFFER_TOO_SMALL && tiny_buffer_get[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC9 - Expected ret XML_UTIL_ERROR_BUFFER_TOO_SMALL (%d), empty buf. Got %d, '%s'\n", XML_UTIL_ERROR_BUFFER_TOO_SMALL, res, tiny_buffer_get); assert(0); }

    // Test case 10: Bad parameters (NULL xml_content)
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(NULL, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_ERROR_BAD_PARAMS && buffer[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC10 - Expected ret XML_UTIL_ERROR_BAD_PARAMS (%d), empty buf. Got %d, '%s'\n", XML_UTIL_ERROR_BAD_PARAMS, res, buffer); assert(0); }

    // Test case 11: Bad parameters (NULL tag_name)
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml1, NULL, buffer, sizeof(buffer));
    if (res == XML_UTIL_ERROR_BAD_PARAMS && buffer[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC11 - Expected ret XML_UTIL_ERROR_BAD_PARAMS (%d), empty buf. Got %d, '%s'\n", XML_UTIL_ERROR_BAD_PARAMS, res, buffer); assert(0); }

    // Test case 12: Bad parameters (NULL buffer)
    tests_run++;
    // No buffer check for this one as it's NULL
    res = get_tag_string(xml1, "test", NULL, 10);
    if (res == XML_UTIL_ERROR_BAD_PARAMS) tests_passed++;
    else { printf("FAIL: get_tag_string TC12 - Expected ret XML_UTIL_ERROR_BAD_PARAMS (%d). Got %d\n", XML_UTIL_ERROR_BAD_PARAMS, res); assert(0); }

    // Test case 13: Bad parameters (zero buffer_size)
    tests_run++;
    // No buffer check for this one as it's zero size
    res = get_tag_string(xml1, "test", buffer, 0);
    if (res == XML_UTIL_ERROR_BAD_PARAMS) tests_passed++;
    else { printf("FAIL: get_tag_string TC13 - Expected ret XML_UTIL_ERROR_BAD_PARAMS (%d). Got %d\n", XML_UTIL_ERROR_BAD_PARAMS, res); assert(0); }


    printf("get_tag_string tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}

static int test_set_tag_string() {
    printf("Running tests for set_tag_string...\n");
    char xml_buffer[256];
    int tests_passed = 0;
    int tests_run = 0;
    char original_xml_content[256];
    int set_res;

    tests_run++;
    strcpy(xml_buffer, "<root><item>old</item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "new");
    if (set_res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item>new</item></root>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC1. Got %d\n", set_res); assert(0); }

    tests_run++;
    strcpy(xml_buffer, "<root><item>longer</item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "short");
    if (set_res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item>short</item></root>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC2. Got %d\n", set_res); assert(0); }

    tests_run++;
    strcpy(xml_buffer, "<root><item>short</item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "much_longer_value");
    if (set_res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item>much_longer_value</item></root>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC3. Got %d\n", set_res); assert(0); }

    tests_run++;
    strcpy(xml_buffer, "<root><item>data</item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "");
    if (set_res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item></item></root>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC4. Got %d\n", set_res); assert(0); }

    tests_run++;
    strcpy(xml_buffer, "<root><item></item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "filled");
    if (set_res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item>filled</item></root>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC5. Got %d\n", set_res); assert(0); }

    tests_run++;
    strcpy(xml_buffer, "<root><item>value</item></root>");
    strcpy(original_xml_content, xml_buffer);
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "nonexistent", "new");
    // Expect TAG_NOT_FOUND from updated set_tag_string
    if (set_res == XML_UTIL_ERROR_TAG_NOT_FOUND && strcmp(xml_buffer, original_xml_content) == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC6. Expected %d. Got %d\n", XML_UTIL_ERROR_TAG_NOT_FOUND, set_res); assert(0); }

    tests_run++;
    strcpy(xml_buffer, "<data><val1>abc</val1><val2>def</val2></data>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "val1", "xyz");
    if (set_res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<data><val1>xyz</val1><val2>def</val2></data>") == 0) {
        set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "val2", "jkl");
        if (set_res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<data><val1>xyz</val1><val2>jkl</val2></data>") == 0) tests_passed++;
        else { printf("FAIL: set_tag_string TC7b. Got %d\n", set_res); assert(0); }
    } else { printf("FAIL: set_tag_string TC7a. Got %d\n", set_res); assert(0); }


    tests_run++;
    strcpy(xml_buffer, "<first>one</first><second>two</second>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "second", "new_two");
    if (set_res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<first>one</first><second>new_two</second>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC8. Got %d\n", set_res); assert(0); }

    printf("Testing set_tag_string overflow conditions...\n");
    char tight_buffer[30];

    tests_run++;
    strcpy(tight_buffer, "<tag>short</tag>");
    strcpy(original_xml_content, tight_buffer);
    set_res = set_tag_string(tight_buffer, sizeof(tight_buffer), "tag", "this_is_a_very_long_value");
    if (set_res == -2 && strcmp(tight_buffer, original_xml_content) == 0) tests_passed++; // -2 is set_tag_string's specific overflow
    else { printf("FAIL: set_tag_string Overflow TC1. Got %d\n", set_res); assert(0); }

    tests_run++;
    strcpy(tight_buffer, "<tag>val</tag>");
    set_res = set_tag_string(tight_buffer, sizeof(tight_buffer), "tag", "value_fits_tight");
    if (set_res == XML_UTIL_SUCCESS && strcmp(tight_buffer, "<tag>value_fits_tight</tag>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string Overflow TC2. Got %d\n", set_res); assert(0); }

    char tiny_buf[10];
    tests_run++;
    strcpy(tiny_buf, "<t>v</t>");
    strcpy(original_xml_content, tiny_buf);
    set_res = set_tag_string(tiny_buf, sizeof(tiny_buf), "t", "123");
    if (set_res == -2 && strcmp(tiny_buf, original_xml_content) == 0) tests_passed++; // -2 specific overflow
    else { printf("FAIL: set_tag_string Overflow TC3 (tiny_buf '123'). Got %d\n", set_res); assert(0); }

    char eleven_buf[11];
    tests_run++;
    strcpy(eleven_buf, "<t>v</t>");
    set_res = set_tag_string(eleven_buf, sizeof(eleven_buf), "t", "123");
    if (set_res == XML_UTIL_SUCCESS && strcmp(eleven_buf, "<t>123</t>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string Exact Fit TC (eleven_buf '123'). Got %d\n", set_res); assert(0); }

    tests_run++;
    strcpy(tiny_buf, "<t>v</t>");
    strcpy(original_xml_content, tiny_buf);
    set_res = set_tag_string(tiny_buf, sizeof(tiny_buf), "t", "1234");
    if (set_res == -2 && strcmp(tiny_buf, original_xml_content) == 0) tests_passed++; // -2 specific overflow
    else { printf("FAIL: set_tag_string Overflow TC4 (tiny_buf '1234'). Got %d\n", set_res); assert(0); }


    printf("set_tag_string tests (including overflow): %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}


static int test_xml_modification_scenario() {
    printf("Running tests for test_xml_modification_scenario...\n");
    int tests_passed = 0;
    int tests_run = 0;
    int op_res;

    char xml_buffer[1024] =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<sys_prod type=\"complex\" access=\"510\">\n"
        "  <version type=\"unsignedInt\" length=\"4\" access=\"510\">5</version>\n"
        "  <eeprom_version type=\"unsignedShort\" length=\"2\" access=\"510\">1</eeprom_version>\n"
        "  <product_area type=\"unsignedInt\" length=\"4\" access=\"510\">2</product_area>\n"
        "  <game_region type=\"unsignedInt\" length=\"4\" access=\"510\">2</game_region>\n"
        "  <ntsc_pal type=\"string\" length=\"5\" access=\"510\">NTSC</ntsc_pal>\n"
        "  <5ghz_country_code type=\"string\" length=\"4\" access=\"510\">Q2</5ghz_country_code>\n"
        "  <5ghz_country_code_revision type=\"unsignedByte\" length=\"1\" access=\"510\">7</5ghz_country_code_revision>\n"
        "  <code_id type=\"string\" length=\"8\" access=\"510\">FW</code_id>\n"
        "  <serial_id type=\"string\" length=\"12\" access=\"510\">409950593</serial_id>\n"
        "  <model_number type=\"string\" length=\"16\" access=\"510\">WUP-101(02)</model_number>\n"
        "</sys_prod>";

    char temp_value_buffer[32];
    const char* initial_pa = "2";
    const char* initial_gr = "2";
    const char* new_pa_str = "4";
    const char* new_gr_str = "1";

    const char* expected_final_xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<sys_prod type=\"complex\" access=\"510\">\n"
        "  <version type=\"unsignedInt\" length=\"4\" access=\"510\">5</version>\n"
        "  <eeprom_version type=\"unsignedShort\" length=\"2\" access=\"510\">1</eeprom_version>\n"
        "  <product_area type=\"unsignedInt\" length=\"4\" access=\"510\">4</product_area>\n"
        "  <game_region type=\"unsignedInt\" length=\"4\" access=\"510\">1</game_region>\n"
        "  <ntsc_pal type=\"string\" length=\"5\" access=\"510\">NTSC</ntsc_pal>\n"
        "  <5ghz_country_code type=\"string\" length=\"4\" access=\"510\">Q2</5ghz_country_code>\n"
        "  <5ghz_country_code_revision type=\"unsignedByte\" length=\"1\" access=\"510\">7</5ghz_country_code_revision>\n"
        "  <code_id type=\"string\" length=\"8\" access=\"510\">FW</code_id>\n"
        "  <serial_id type=\"string\" length=\"12\" access=\"510\">409950593</serial_id>\n"
        "  <model_number type=\"string\" length=\"16\" access=\"510\">WUP-101(02)</model_number>\n"
        "</sys_prod>";

    // 1. Get and validate initial product_area
    tests_run++;
    op_res = get_tag_string(xml_buffer, "product_area", temp_value_buffer, sizeof(temp_value_buffer));
    int pa_valid_initial = (op_res == XML_UTIL_SUCCESS && strcmp(temp_value_buffer, initial_pa) == 0);
    int pa_is_allowed_type = pa_valid_initial && (strcmp(temp_value_buffer, "1") == 0 || strcmp(temp_value_buffer, "2") == 0 || strcmp(temp_value_buffer, "4") == 0 || strcmp(temp_value_buffer, "119") == 0);
    if (pa_is_allowed_type) {
        tests_passed++;
    } else {
        printf("FAIL: Scenario - Get/Validate initial product_area. Ret: %d, Got_Val: '%s', Expected_Initial: '%s'\n", op_res, temp_value_buffer, initial_pa);
        assert(0);
    }

    // 2. Get and validate initial game_region
    tests_run++;
    op_res = get_tag_string(xml_buffer, "game_region", temp_value_buffer, sizeof(temp_value_buffer));
    int gr_valid_initial = (op_res == XML_UTIL_SUCCESS && strcmp(temp_value_buffer, initial_gr) == 0);
    int gr_is_allowed_type = gr_valid_initial && (strcmp(temp_value_buffer, "1") == 0 || strcmp(temp_value_buffer, "2") == 0 || strcmp(temp_value_buffer, "4") == 0 || strcmp(temp_value_buffer, "119") == 0);
    if (gr_is_allowed_type) {
        tests_passed++;
    } else {
        printf("FAIL: Scenario - Get/Validate initial game_region. Ret: %d, Got_Val: '%s', Expected_Initial: '%s'\n", op_res, temp_value_buffer, initial_gr);
        assert(0);
    }

    // 3. Set new product_area
    tests_run++;
    op_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "product_area", new_pa_str);
    if (op_res == XML_UTIL_SUCCESS) {
         tests_passed++;
    } else { printf("FAIL: Scenario - Set new product_area to %s, got %d\n", new_pa_str, op_res); assert(0); }

    // 4. Set new game_region
    tests_run++;
    op_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "game_region", new_gr_str);
    if (op_res == XML_UTIL_SUCCESS) {
        tests_passed++;
    } else { printf("FAIL: Scenario - Set new game_region to %s, got %d\n", new_gr_str, op_res); assert(0); }

    // 5. Verify final XML content
    tests_run++;
    if (strcmp(xml_buffer, expected_final_xml) == 0) {
        tests_passed++;
    } else {
        printf("FAIL: Scenario - Final XML does not match expected.\n");
        // printf("=== Start Modified XML ===\n%s\n=== End Modified XML ===\n", xml_buffer);
        // printf("=== Start Expected XML ===\n%s\n=== End Expected XML ===\n", expected_final_xml);
        assert(0);
    }

    printf("test_xml_modification_scenario tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}


// Main function to run all test suites
int main() {
    printf("Starting sysprod XML helper tests...\n");
    int all_suites_passed = 1;

    if (!test_find_tag_value_start()) {
        all_suites_passed = 0;
        printf("FAIL: test_find_tag_value_start suite failed.\n");
    }
    if (!test_get_tag_string()) {
        all_suites_passed = 0;
        printf("FAIL: test_get_tag_string suite failed.\n");
    }
    if (!test_set_tag_string()) {
        all_suites_passed = 0;
        printf("FAIL: test_set_tag_string suite failed.\n");
    }
    if (!test_xml_modification_scenario()) {
        all_suites_passed = 0;
        printf("FAIL: test_xml_modification_scenario suite failed.\n");
    }

    if (all_suites_passed) {
        printf("\nAll sysprod XML helper tests PASSED.\n");
        return 0; // Success
    } else {
        printf("\nOne or more sysprod XML helper tests FAILED.\n");
        return 1; // Failure
    }
}
