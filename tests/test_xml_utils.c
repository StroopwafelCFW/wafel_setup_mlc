#include <stdio.h>
#include <string.h>
#include <assert.h> // For assert()

// For the functions we are testing:
// find_tag_value_start, get_tag_string, set_tag_string
// These are declared in sysprod.h.
// We assume that when compiling these tests, the include paths are set up
// correctly to find "xml_utils.h" and its dependencies (like <stddef.h>).
#include "xml_utils.h" // For XML helper function declarations

// Test suite for find_tag_value_start (no changes needed for this function)
static int test_find_tag_value_start() {
    printf("Running tests for find_tag_value_start...\n");
    int tests_passed = 0;
    int tests_run = 0;

    const char* xml1 = "<test>value</test>";
    tests_run++;
    const char* result1 = find_tag_value_start(xml1, "test");
    assert(result1 != NULL && strcmp(result1, "value</test>") == 0);
    if (result1 != NULL && strcmp(result1, "value</test>") == 0) tests_passed++;

    const char* xml2 = "<outer><test>val</test></outer>";
    tests_run++;
    const char* result2 = find_tag_value_start(xml2, "test");
    assert(result2 != NULL && strcmp(result2, "val</test></outer>") == 0);
    if (result2 != NULL && strcmp(result2, "val</test></outer>") == 0) tests_passed++;

    const char* xml3 = "<test></test>";
    tests_run++;
    const char* result3 = find_tag_value_start(xml3, "test");
    assert(result3 != NULL && strcmp(result3, "</test>") == 0);
    if (result3 != NULL && strcmp(result3, "</test>") == 0) tests_passed++;

    const char* xml4 = "<anothertag>value</anothertag><test>value2</test>";
    tests_run++;
    const char* result4 = find_tag_value_start(xml4, "test");
    assert(result4 != NULL && strcmp(result4, "value2</test>") == 0);
    if (result4 != NULL && strcmp(result4, "value2</test>") == 0) tests_passed++;

    tests_run++;
    const char* result5 = find_tag_value_start(xml1, "nonexistent");
    assert(result5 == NULL);
    if (result5 == NULL) tests_passed++;

    tests_run++;
    const char* result6 = find_tag_value_start(NULL, "test");
    assert(result6 == NULL);
    if (result6 == NULL) tests_passed++;

    tests_run++;
    const char* result7 = find_tag_value_start(xml1, NULL);
    assert(result7 == NULL);
    if (result7 == NULL) tests_passed++;

    printf("find_tag_value_start tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}

// Test suite for get_tag_string (updated for new behavior)
static int test_get_tag_string() {
    printf("Running tests for get_tag_string...\n");
    char buffer[50];
    int tests_passed = 0;
    int tests_run = 0;

    // Test case 1: Simple extraction (value fits)
    const char* xml1 = "<test>value</test>"; // value length 5
    tests_run++;
    buffer[0] = 'X'; // Pre-fill to check it gets overwritten by empty string on error
    assert(get_tag_string(xml1, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "value") == 0);
    if (get_tag_string(xml1, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "value") == 0) tests_passed++;

    // Test case 2: Nested tag extraction (value fits)
    const char* xml2 = "<outer><test>val</test></outer>"; // val length 3
    tests_run++;
    assert(get_tag_string(xml2, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "val") == 0);
    if (get_tag_string(xml2, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "val") == 0) tests_passed++;

    // Test case 3: Empty value tag
    const char* xml3 = "<test></test>";
    tests_run++;
    assert(get_tag_string(xml3, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "") == 0);
    if (get_tag_string(xml3, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "") == 0) tests_passed++;

    // Test case 4: Value too long for buffer (NEW BEHAVIOR: expect NULL)
    const char* xml_long_val = "<item>verylongvalue</item>"; // length 15
    char small_buf[10]; // Can hold 9 chars + null
    tests_run++;
    small_buf[0] = 'X'; // Pre-fill to check for '\0'
    assert(get_tag_string(xml_long_val, "item", small_buf, sizeof(small_buf)) == NULL && small_buf[0] == '\0');
    if (get_tag_string(xml_long_val, "item", small_buf, sizeof(small_buf)) == NULL && small_buf[0] == '\0') tests_passed++;

    // Test case 5: Value fits exactly in buffer
    const char* xml_exact_val = "<item>exactfit</item>"; // length 8
    char exact_buf[9]; // Can hold "exactfit" (8 chars) + null terminator
    tests_run++;
    exact_buf[0] = 'X';
    assert(get_tag_string(xml_exact_val, "item", exact_buf, sizeof(exact_buf)) != NULL && strcmp(exact_buf, "exactfit") == 0);
    if (get_tag_string(xml_exact_val, "item", exact_buf, sizeof(exact_buf)) != NULL && strcmp(exact_buf, "exactfit") == 0) tests_passed++;

    // Test case 6: Value just one char too long for buffer (value len == buffer size)
    const char* xml_just_too_long = "<item>toolonggg</item>"; // length 9
    char just_too_long_buf[9]; // Can hold 8 chars + null
    tests_run++;
    just_too_long_buf[0] = 'X';
    assert(get_tag_string(xml_just_too_long, "item", just_too_long_buf, sizeof(just_too_long_buf)) == NULL && just_too_long_buf[0] == '\0');
    if (get_tag_string(xml_just_too_long, "item", just_too_long_buf, sizeof(just_too_long_buf)) == NULL && just_too_long_buf[0] == '\0') tests_passed++;

    // Test case 7: Non-existent tag
    tests_run++;
    buffer[0] = 'X';
    assert(get_tag_string(xml1, "nonexistent", buffer, sizeof(buffer)) == NULL && buffer[0] == '\0');
    if (get_tag_string(xml1, "nonexistent", buffer, sizeof(buffer)) == NULL && buffer[0] == '\0') tests_passed++;

    // Test case 8: Tag exists but no closing tag
    const char* xml_no_close = "<test>value_no_close";
    tests_run++;
    buffer[0] = 'X';
    assert(get_tag_string(xml_no_close, "test", buffer, sizeof(buffer)) == NULL && buffer[0] == '\0');
    if (get_tag_string(xml_no_close, "test", buffer, sizeof(buffer)) == NULL && buffer[0] == '\0') tests_passed++;

    // Test case 9: Buffer size 1 (can only hold null terminator)
    const char* xml_val_a = "<test>a</test>"; // value "a" length 1
    char tiny_buffer_get[1]; // Renamed to avoid conflict with set_tag_string tests
    tests_run++;
    tiny_buffer_get[0] = 'X';
    // Value "a" (len 1) needs buffer of size 2. Buffer of size 1 is too small.
    assert(get_tag_string(xml_val_a, "test", tiny_buffer_get, sizeof(tiny_buffer_get)) == NULL && tiny_buffer_get[0] == '\0');
    if (get_tag_string(xml_val_a, "test", tiny_buffer_get, sizeof(tiny_buffer_get)) == NULL && tiny_buffer_get[0] == '\0') tests_passed++;

    printf("get_tag_string tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}

// Test suite for set_tag_string (updated for new signature and overflow tests)
static int test_set_tag_string() {
    printf("Running tests for set_tag_string...\n");
    char xml_buffer[256]; // Mutable buffer for testing
    int tests_passed = 0;
    int tests_run = 0;
    char original_xml_content[256]; // For checking buffer unchanged on error
    int set_res; // To store result of set_tag_string for checking

    // Test case 1: New value same length
    tests_run++;
    strcpy(xml_buffer, "<root><item>old</item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "new");
    assert(set_res == 0 && strcmp(xml_buffer, "<root><item>new</item></root>") == 0);
    if (set_res == 0 && strcmp(xml_buffer, "<root><item>new</item></root>") == 0) tests_passed++;

    // Test case 2: New value shorter
    tests_run++;
    strcpy(xml_buffer, "<root><item>longer</item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "short");
    assert(set_res == 0 && strcmp(xml_buffer, "<root><item>short</item></root>") == 0);
    if (set_res == 0 && strcmp(xml_buffer, "<root><item>short</item></root>") == 0) tests_passed++;

    // Test case 3: New value longer (still fits in large buffer)
    tests_run++;
    strcpy(xml_buffer, "<root><item>short</item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "much_longer_value");
    assert(set_res == 0 && strcmp(xml_buffer, "<root><item>much_longer_value</item></root>") == 0);
    if (set_res == 0 && strcmp(xml_buffer, "<root><item>much_longer_value</item></root>") == 0) tests_passed++;

    // Test case 4: Empty new value
    tests_run++;
    strcpy(xml_buffer, "<root><item>data</item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "");
    assert(set_res == 0 && strcmp(xml_buffer, "<root><item></item></root>") == 0);
    if (set_res == 0 && strcmp(xml_buffer, "<root><item></item></root>") == 0) tests_passed++;

    // Test case 5: Setting value for an initially empty tag
    tests_run++;
    strcpy(xml_buffer, "<root><item></item></root>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "filled");
    assert(set_res == 0 && strcmp(xml_buffer, "<root><item>filled</item></root>") == 0);
    if (set_res == 0 && strcmp(xml_buffer, "<root><item>filled</item></root>") == 0) tests_passed++;

    // Test case 6: Tag not found
    tests_run++;
    strcpy(xml_buffer, "<root><item>value</item></root>");
    strcpy(original_xml_content, xml_buffer); // Save original
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "nonexistent", "new");
    assert(set_res == -1 && strcmp(xml_buffer, original_xml_content) == 0);
    if (set_res == -1 && strcmp(xml_buffer, original_xml_content) == 0) tests_passed++;

    // Test case 7: Multiple tags, ensure only correct one is changed
    tests_run++;
    strcpy(xml_buffer, "<data><val1>abc</val1><val2>def</val2></data>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "val1", "xyz");
    assert(set_res == 0 && strcmp(xml_buffer, "<data><val1>xyz</val1><val2>def</val2></data>") == 0);
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "val2", "jkl"); // Second modification
    assert(set_res == 0 && strcmp(xml_buffer, "<data><val1>xyz</val1><val2>jkl</val2></data>") == 0);
    if (strcmp(xml_buffer, "<data><val1>xyz</val1><val2>jkl</val2></data>") == 0) tests_passed++;


    // Test case 8: Tag at the end of the document
    tests_run++;
    strcpy(xml_buffer, "<first>one</first><second>two</second>");
    set_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "second", "new_two");
    assert(set_res == 0 && strcmp(xml_buffer, "<first>one</first><second>new_two</second>") == 0);
    if (set_res == 0 && strcmp(xml_buffer, "<first>one</first><second>new_two</second>") == 0) tests_passed++;

    printf("Testing set_tag_string overflow conditions...\n");
    char tight_buffer[30];

    // Overflow Test 1: New value makes it overflow
    tests_run++;
    strcpy(tight_buffer, "<tag>short</tag>");
    strcpy(original_xml_content, tight_buffer);
    set_res = set_tag_string(tight_buffer, sizeof(tight_buffer), "tag", "this_is_a_very_long_value");
    assert(set_res == -2 && strcmp(tight_buffer, original_xml_content) == 0);
    if (set_res == -2 && strcmp(tight_buffer, original_xml_content) == 0) tests_passed++;

    // Overflow Test 2: New value makes it fit exactly (original was correct)
    tests_run++;
    strcpy(tight_buffer, "<tag>val</tag>");
    set_res = set_tag_string(tight_buffer, sizeof(tight_buffer), "tag", "value_fits_tight");
    assert(set_res == 0 && strcmp(tight_buffer, "<tag>value_fits_tight</tag>") == 0);
    if (set_res == 0 && strcmp(tight_buffer, "<tag>value_fits_tight</tag>") == 0) tests_passed++;

    // Corrected Overflow Test: value "123" in tiny_buf[10] IS an overflow
    char tiny_buf[10];
    tests_run++;
    strcpy(tiny_buf, "<t>v</t>");
    strcpy(original_xml_content, tiny_buf);
    // Target XML: "<t>123</t>" (10 chars). Needs 11 bytes for null terminator. sizeof(tiny_buf) is 10. Overflow.
    set_res = set_tag_string(tiny_buf, sizeof(tiny_buf), "t", "123");
    assert(set_res == -2 && strcmp(tiny_buf, original_xml_content) == 0);
    if (set_res == -2 && strcmp(tiny_buf, original_xml_content) == 0) tests_passed++;

    // New test: Exact fit for "<t>123</t>" using a buffer of size 11
    char eleven_buf[11];
    tests_run++;
    strcpy(eleven_buf, "<t>v</t>");
    // Target XML: "<t>123</t>" (10 chars). Needs 11 bytes for null terminator. sizeof(eleven_buf) is 11. Fits.
    set_res = set_tag_string(eleven_buf, sizeof(eleven_buf), "t", "123");
    assert(set_res == 0 && strcmp(eleven_buf, "<t>123</t>") == 0);
    if (set_res == 0 && strcmp(eleven_buf, "<t>123</t>") == 0) tests_passed++;

    // Overflow Test : New value *just* overflows tiny_buf[10]
    tests_run++;
    strcpy(tiny_buf, "<t>v</t>");
    strcpy(original_xml_content, tiny_buf);
    // New value: "1234" (4 chars). Old "v" (1 char). diff = 3.
    // Projected total len = 7 + 3 = 10. (projected_len+1) = 11. 11 > 10, so overflow.
    set_res = set_tag_string(tiny_buf, sizeof(tiny_buf), "t", "1234");
    assert(set_res == -2 && strcmp(tiny_buf, original_xml_content) == 0);
    if (set_res == -2 && strcmp(tiny_buf, original_xml_content) == 0) tests_passed++;


    printf("set_tag_string tests (including overflow): %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}


static int test_xml_modification_scenario() {
    printf("Running tests for test_xml_modification_scenario...\n");
    int tests_passed = 0;
    int tests_run = 0;
    int op_res; // For results of set/get operations
    char* get_ptr_res; // For result of get_tag_string

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
    get_ptr_res = get_tag_string(xml_buffer, "product_area", temp_value_buffer, sizeof(temp_value_buffer));
    int pa_valid_initial = (get_ptr_res != NULL && strcmp(temp_value_buffer, initial_pa) == 0);
    int pa_is_allowed_val = pa_valid_initial && (strcmp(temp_value_buffer, "1") == 0 || strcmp(temp_value_buffer, "2") == 0 || strcmp(temp_value_buffer, "4") == 0 || strcmp(temp_value_buffer, "119") == 0);
    assert(pa_is_allowed_val);
    if (pa_is_allowed_val) {
        tests_passed++;
    } else { printf("FAIL: Scenario - Get/Validate initial product_area. Got: '%s', Expected: '%s', Valid Check: %d\n", temp_value_buffer, initial_pa, pa_valid_initial); }

    // 2. Get and validate initial game_region
    tests_run++;
    get_ptr_res = get_tag_string(xml_buffer, "game_region", temp_value_buffer, sizeof(temp_value_buffer));
    int gr_valid_initial = (get_ptr_res != NULL && strcmp(temp_value_buffer, initial_gr) == 0);
    int gr_is_allowed_val = gr_valid_initial && (strcmp(temp_value_buffer, "1") == 0 || strcmp(temp_value_buffer, "2") == 0 || strcmp(temp_value_buffer, "4") == 0 || strcmp(temp_value_buffer, "119") == 0);
    assert(gr_is_allowed_val);
    if (gr_is_allowed_val) {
        tests_passed++;
    } else { printf("FAIL: Scenario - Get/Validate initial game_region. Got: '%s', Expected: '%s', Valid Check: %d\n", temp_value_buffer, initial_gr, gr_valid_initial); }

    // 3. Set new product_area
    tests_run++;
    op_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "product_area", new_pa_str);
    assert(op_res == 0);
    if (op_res == 0) {
         tests_passed++;
    } else { printf("FAIL: Scenario - Set new product_area to %s, got %d\n", new_pa_str, op_res); }

    // 4. Set new game_region
    tests_run++;
    op_res = set_tag_string(xml_buffer, sizeof(xml_buffer), "game_region", new_gr_str);
    assert(op_res == 0);
    if (op_res == 0) {
        tests_passed++;
    } else { printf("FAIL: Scenario - Set new game_region to %s, got %d\n", new_gr_str, op_res); }

    // 5. Verify final XML content
    tests_run++;
    // printf("=== Start Modified XML ===\n%s\n=== End Modified XML ===\n", xml_buffer); // For debugging
    // printf("=== Start Expected XML ===\n%s\n=== End Expected XML ===\n", expected_final_xml); // For debugging
    assert(strcmp(xml_buffer, expected_final_xml) == 0);
    if (strcmp(xml_buffer, expected_final_xml) == 0) {
        tests_passed++;
    } else { printf("FAIL: Scenario - Final XML does not match expected.\n"); }

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
    if (!test_xml_modification_scenario()) { // Add this call
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
