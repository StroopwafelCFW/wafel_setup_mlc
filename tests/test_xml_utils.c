#include <stdio.h>
#include <string.h>
#include <assert.h> // For assert()
#include <ctype.h>  // For isspace, though not directly used in tests, good for context

#include "xml_utils.h" // For XML helper function declarations and error codes

static int test_find_tag_value_start() {
    printf("Running tests for find_tag_value_start...\n");
    int tests_passed = 0;
    int tests_run = 0;
    const char* result_ptr;

    // Basic tests (from before)
    const char* xml1 = "<test>value</test>";
    tests_run++;
    result_ptr = find_tag_value_start(xml1, "test");
    if (result_ptr != NULL && strcmp(result_ptr, "value</test>") == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC1. Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml2 = "<outer><test>val</test></outer>";
    tests_run++;
    result_ptr = find_tag_value_start(xml2, "test");
    if (result_ptr != NULL && strcmp(result_ptr, "val</test></outer>") == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC2. Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml3 = "<test></test>";
    tests_run++;
    result_ptr = find_tag_value_start(xml3, "test");
    if (result_ptr != NULL && strcmp(result_ptr, "</test>") == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC3. Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml4 = "<anothertag>value</anothertag><test>value2</test>";
    tests_run++;
    result_ptr = find_tag_value_start(xml4, "test");
    if (result_ptr != NULL && strcmp(result_ptr, "value2</test>") == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC4. Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    tests_run++;
    result_ptr = find_tag_value_start(xml1, "nonexistent");
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC5 (nonexistent). Got: %s\n", result_ptr); assert(0); }

    tests_run++;
    result_ptr = find_tag_value_start(NULL, "test");
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC6 (NULL xml). Got: %s\n", result_ptr); assert(0); }

    tests_run++;
    result_ptr = find_tag_value_start(xml1, NULL);
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC7 (NULL tag). Got: %s\n", result_ptr); assert(0); }

    tests_run++; // Empty tag name
    result_ptr = find_tag_value_start(xml1, "");
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_tag_value_start TC8 (Empty tag name). Got: %s\n", result_ptr); assert(0); }


    printf("Testing find_tag_value_start with attributes...\n");
    const char* xml_attr1 = "<tag attr=\"val\">value_here</tag>";
    tests_run++;
    result_ptr = find_tag_value_start(xml_attr1, "tag");
    if (result_ptr != NULL && strncmp(result_ptr, "value_here", strlen("value_here")) == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start attr TC1. Expected 'value_here', Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml_attr2 = "<tag type=\"complex\" access=\"510\">  spaced_value</tag>";
    tests_run++;
    result_ptr = find_tag_value_start(xml_attr2, "tag");
    if (result_ptr != NULL && strncmp(result_ptr, "  spaced_value", strlen("  spaced_value")) == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start attr TC2. Expected '  spaced_value', Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml_attr3 = "<tag attr1='val1' attr2=\"val2\">value</tag>";
    tests_run++;
    result_ptr = find_tag_value_start(xml_attr3, "tag");
    if (result_ptr != NULL && strncmp(result_ptr, "value", strlen("value")) == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start attr TC3. Expected 'value', Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml_attr_no_space = "<tag attr=\"val\"another='foo\'>value</tag>";
    tests_run++;
    result_ptr = find_tag_value_start(xml_attr_no_space, "tag");
    if (result_ptr != NULL && strncmp(result_ptr, "value", strlen("value")) == 0) tests_passed++;
    else { printf("FAIL: find_tag_value_start attr TC4 (no space). Expected 'value', Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml_self_closing_attr = "<tag attr=\"val\"/>";
    tests_run++;
    result_ptr = find_tag_value_start(xml_self_closing_attr, "tag");
    if (result_ptr != NULL && *(result_ptr-1) == '>' && *result_ptr == '\0') tests_passed++;
    else { printf("FAIL: find_tag_value_start attr TC5 (self-closing w attr). Expected empty. Got: '%s'\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml_self_closing_direct = "<mptag />";
    tests_run++;
    result_ptr = find_tag_value_start(xml_self_closing_direct, "mptag");
    if (result_ptr != NULL && *result_ptr == '\0' && *(result_ptr-1) == '>') tests_passed++;
    else { printf("FAIL: find_tag_value_start attr TC6 (self-closing direct). Expected empty. Got: '%s'\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml_prefix_match = "<tag_other>value</tag_other>";
    tests_run++;
    result_ptr = find_tag_value_start(xml_prefix_match, "tag"); // Should not match "tag_other"
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_tag_value_start attr TC8 (prefix mismatch). Expected NULL. Got: %s\n", result_ptr); assert(0); }


    printf("find_tag_value_start tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}

static int test_find_closing_tag_pos() {
    printf("Running tests for find_closing_tag_pos...\n");
    int tests_passed = 0;
    int tests_run = 0;
    const char* result_ptr;

    const char* xml_simple = "value</tag>rest";
    tests_run++;
    result_ptr = find_closing_tag_pos(xml_simple, "tag");
    if (result_ptr != NULL && strcmp(result_ptr, "</tag>rest") == 0) tests_passed++;
    else { printf("FAIL: find_closing_tag_pos TC1. Expected '</tag>rest'. Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml_search_from_start = "<tag>value</tag>rest";
    tests_run++;
    result_ptr = find_closing_tag_pos(xml_search_from_start, "tag");
    if (result_ptr != NULL && strcmp(result_ptr, "</tag>rest") == 0) tests_passed++;
    else { printf("FAIL: find_closing_tag_pos TC2. Expected '</tag>rest'. Got: %s\n", result_ptr ? result_ptr : "NULL"); assert(0); }

    const char* xml_not_found = "value<tag>other</tag>";
    tests_run++;
    result_ptr = find_closing_tag_pos(xml_not_found, "tag_nonexistent");
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_closing_tag_pos TC3. Expected NULL. Got: %s\n", result_ptr); assert(0); }

    tests_run++;
    char long_tag_name[200];
    memset(long_tag_name, 'a', 199);
    long_tag_name[199] = '\0';
    result_ptr = find_closing_tag_pos(xml_simple, long_tag_name);
    if (result_ptr == NULL) tests_passed++;
    else { printf("FAIL: find_closing_tag_pos TC4 (long tag). Expected NULL. Got: %s\n", result_ptr); assert(0); }

    tests_run++;
    result_ptr = find_closing_tag_pos(NULL, "tag");
    if (result_ptr == NULL) tests_passed++; else { printf("FAIL: find_closing_tag_pos TC5 (NULL search_start_pos)\n"); assert(0); }

    tests_run++;
    result_ptr = find_closing_tag_pos(xml_simple, NULL);
    if (result_ptr == NULL) tests_passed++; else { printf("FAIL: find_closing_tag_pos TC6 (NULL tag_name)\n"); assert(0); }

    tests_run++;
    result_ptr = find_closing_tag_pos(xml_simple, "");
    if (result_ptr == NULL) tests_passed++; else { printf("FAIL: find_closing_tag_pos TC7 (Empty tag_name)\n"); assert(0); }

    printf("test_find_closing_tag_pos tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}


static int test_get_tag_string() {
    printf("Running tests for get_tag_string...\n");
    char buffer[50];
    int tests_passed = 0;
    int tests_run = 0;
    int res;

    const char* xml1 = "<test>value</test>";
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml1, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_SUCCESS && strcmp(buffer, "value") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string TC1. Got %d, '%s'\n", res, buffer); assert(0); }

    const char* xml2 = "<outer><test>val</test></outer>";
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml2, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_SUCCESS && strcmp(buffer, "val") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string TC2. Got %d, '%s'\n", res, buffer); assert(0); }

    const char* xml3 = "<test></test>";
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml3, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_SUCCESS && strcmp(buffer, "") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string TC3. Got %d, '%s'\n", res, buffer); assert(0); }

    const char* xml_long_val = "<item>verylongvalue</item>";
    char small_buf[10];
    tests_run++;
    small_buf[0] = 'X';
    res = get_tag_string(xml_long_val, "item", small_buf, sizeof(small_buf));
    if (res == XML_UTIL_ERROR_BUFFER_TOO_SMALL && small_buf[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC4. Got %d, '%s'\n", res, small_buf); assert(0); }

    const char* xml_exact_val = "<item>exactfit</item>";
    char exact_buf[9];
    tests_run++;
    exact_buf[0] = 'X';
    res = get_tag_string(xml_exact_val, "item", exact_buf, sizeof(exact_buf));
    if (res == XML_UTIL_SUCCESS && strcmp(exact_buf, "exactfit") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string TC5. Got %d, '%s'\n", res, exact_buf); assert(0); }

    const char* xml_just_too_long = "<item>toolonggg</item>";
    char just_too_long_buf[9];
    tests_run++;
    just_too_long_buf[0] = 'X';
    res = get_tag_string(xml_just_too_long, "item", just_too_long_buf, sizeof(just_too_long_buf));
    if (res == XML_UTIL_ERROR_BUFFER_TOO_SMALL && just_too_long_buf[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC6. Got %d, '%s'\n", res, just_too_long_buf); assert(0); }

    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml1, "nonexistent", buffer, sizeof(buffer));
    if (res == XML_UTIL_ERROR_TAG_NOT_FOUND && buffer[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC7. Got %d, '%s'\n", res, buffer); assert(0); }

    const char* xml_no_close = "<test>value_no_close";
    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml_no_close, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_ERROR_MALFORMED_XML && buffer[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC8. Got %d, '%s'\n", res, buffer); assert(0); }

    const char* xml_val_a = "<test>a</test>";
    char tiny_buffer_get[1];
    tests_run++;
    tiny_buffer_get[0] = 'X';
    res = get_tag_string(xml_val_a, "test", tiny_buffer_get, sizeof(tiny_buffer_get));
    if (res == XML_UTIL_ERROR_BUFFER_TOO_SMALL && tiny_buffer_get[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC9. Got %d, '%s'\n", res, tiny_buffer_get); assert(0); }

    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(NULL, "test", buffer, sizeof(buffer));
    if (res == XML_UTIL_ERROR_BAD_PARAMS && buffer[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC10. Got %d, '%s'\n", res, buffer); assert(0); }

    tests_run++;
    buffer[0] = 'X';
    res = get_tag_string(xml1, NULL, buffer, sizeof(buffer));
    if (res == XML_UTIL_ERROR_BAD_PARAMS && buffer[0] == '\0') tests_passed++;
    else { printf("FAIL: get_tag_string TC11. Got %d, '%s'\n", res, buffer); assert(0); }

    tests_run++;
    res = get_tag_string(xml1, "test", NULL, 10);
    if (res == XML_UTIL_ERROR_BAD_PARAMS) tests_passed++;
    else { printf("FAIL: get_tag_string TC12. Got %d\n", res); assert(0); }

    tests_run++;
    res = get_tag_string(xml1, "test", buffer, 0);
    if (res == XML_UTIL_ERROR_BAD_PARAMS) tests_passed++; // Buffer itself is valid, but size 0.
    else { printf("FAIL: get_tag_string TC13. Got %d\n", res); assert(0); }

    printf("Testing get_tag_string with attributes...\n");
    const char* xml_ga1 = "<tag1 attr='foo'>value1</tag1>";
    char buf_ga1[20];
    tests_run++;
    res = get_tag_string(xml_ga1, "tag1", buf_ga1, sizeof(buf_ga1));
    if (res == XML_UTIL_SUCCESS && strcmp(buf_ga1, "value1") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string attr TC1. Expected SUCCESS and 'value1'. Got ret %d, buf '%s'\n", res, buf_ga1); assert(0); }

    const char* xml_ga2 = "<tag2 attr1=\"val1\" attr2='val2'>  value with spaces  </tag2>";
    char buf_ga2[30];
    tests_run++;
    res = get_tag_string(xml_ga2, "tag2", buf_ga2, sizeof(buf_ga2));
    if (res == XML_UTIL_SUCCESS && strcmp(buf_ga2, "  value with spaces  ") == 0) tests_passed++;
    else { printf("FAIL: get_tag_string attr TC2. Expected SUCCESS and '  value with spaces  '. Got ret %d, buf '%s'\n", res, buf_ga2); assert(0); }


    printf("get_tag_string tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}

static int test_set_tag_string() {
    printf("Running tests for set_tag_string...\n");
    char xml_buffer[256];
    int tests_passed = 0;
    int tests_run = 0;
    char original_xml_content[256];
    int res;

    tests_run++;
    strcpy(xml_buffer, "<root><item>old</item></root>");
    res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "new");
    if (res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item>new</item></root>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC1. Got %d\n", res); assert(0); }

    // ... (rest of set_tag_string tests are similar, checking for XML_UTIL_SUCCESS or specific error codes) ...
    // For brevity, assuming the rest of the set_tag_string tests are updated similarly to use res and check XML_UTIL_SUCCESS
    // or appropriate XML_UTIL_ERROR_TAG_NOT_FOUND, XML_UTIL_ERROR_BAD_PARAMS, etc.
    // The overflow test expecting -2 should remain -2.

    // Test case 6: Tag not found
    tests_run++;
    strcpy(xml_buffer, "<root><item>value</item></root>");
    strcpy(original_xml_content, xml_buffer);
    res = set_tag_string(xml_buffer, sizeof(xml_buffer), "nonexistent", "new");
    if (res == XML_UTIL_ERROR_TAG_NOT_FOUND && strcmp(xml_buffer, original_xml_content) == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC6. Expected %d. Got %d\n", XML_UTIL_ERROR_TAG_NOT_FOUND, res); assert(0); }

    printf("Testing set_tag_string with attributes...\n");
    char xml_sa1[100];
    tests_run++;
    strcpy(xml_sa1, "<tag1 attr='foo'>old_val</tag1>");
    res = set_tag_string(xml_sa1, sizeof(xml_sa1), "tag1", "new_val");
    if (res == XML_UTIL_SUCCESS && strcmp(xml_sa1, "<tag1 attr='foo'>new_val</tag1>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string attr TC1. Expected SUCCESS. Got ret %d, xml '%s'\n", res, xml_sa1); assert(0); }

    char xml_sa2[100];
    tests_run++;
    strcpy(xml_sa2, "<tag2 attr1=\"val1\" attr2='val2'>  old spaces  </tag2>");
    res = set_tag_string(xml_sa2, sizeof(xml_sa2), "tag2", "new spaces");
    if (res == XML_UTIL_SUCCESS && strcmp(xml_sa2, "<tag2 attr1=\"val1\" attr2='val2'>new spaces</tag2>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string attr TC2. Expected SUCCESS. Got ret %d, xml '%s'\n", res, xml_sa2); assert(0); }


    // Re-pasting the full set_tag_string test with detailed checks for brevity was omitted,
    // but the above examples and the existing structure of the test demonstrate the pattern.
    // Assume all relevant tests in test_set_tag_string are updated.
    // For the actual run, I'll use the previously complete test_set_tag_string and modify it.
    // This is just a placeholder for the thought process.
    // The actual overwrite will contain the fully updated test_set_tag_string.
    // For now, just ensuring the tests_passed count for this placeholder:
    // tests_passed += (8-2+2); // Placeholder for the 8 original tests, minus TC6 now specific, plus 2 attr tests.

    // The below is the correct full set_tag_string for the overwrite
    // (Copied from previous state and modified for error codes)
    strcpy(xml_buffer, "<root><item>longer</item></root>"); tests_run++;
    res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "short");
    if (res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item>short</item></root>") == 0) tests_passed++; else { printf("FAIL: set_tag_string TC2. Got %d\n", res); assert(0); }
    strcpy(xml_buffer, "<root><item>short</item></root>"); tests_run++;
    res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "much_longer_value");
    if (res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item>much_longer_value</item></root>") == 0) tests_passed++; else { printf("FAIL: set_tag_string TC3. Got %d\n", res); assert(0); }
    strcpy(xml_buffer, "<root><item>data</item></root>"); tests_run++;
    res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "");
    if (res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item></item></root>") == 0) tests_passed++; else { printf("FAIL: set_tag_string TC4. Got %d\n", res); assert(0); }
    strcpy(xml_buffer, "<root><item></item></root>"); tests_run++;
    res = set_tag_string(xml_buffer, sizeof(xml_buffer), "item", "filled");
    if (res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<root><item>filled</item></root>") == 0) tests_passed++; else { printf("FAIL: set_tag_string TC5. Got %d\n", res); assert(0); }
    strcpy(xml_buffer, "<data><val1>abc</val1><val2>def</val2></data>"); tests_run++;
    res = set_tag_string(xml_buffer, sizeof(xml_buffer), "val1", "xyz");
    if (res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<data><val1>xyz</val1><val2>def</val2></data>") == 0) {
        res = set_tag_string(xml_buffer, sizeof(xml_buffer), "val2", "jkl");
        if (res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<data><val1>xyz</val1><val2>jkl</val2></data>") == 0) tests_passed++;
        else { printf("FAIL: set_tag_string TC7b. Got %d\n", res); assert(0); }
    } else { printf("FAIL: set_tag_string TC7a. Got %d\n", res); assert(0); }
    strcpy(xml_buffer, "<first>one</first><second>two</second>"); tests_run++;
    res = set_tag_string(xml_buffer, sizeof(xml_buffer), "second", "new_two");
    if (res == XML_UTIL_SUCCESS && strcmp(xml_buffer, "<first>one</first><second>new_two</second>") == 0) tests_passed++;
    else { printf("FAIL: set_tag_string TC8. Got %d\n", res); assert(0); }
    printf("Testing set_tag_string overflow conditions...\n");
    char tight_buffer[30];
    strcpy(tight_buffer, "<tag>short</tag>"); strcpy(original_xml_content, tight_buffer); tests_run++;
    res = set_tag_string(tight_buffer, sizeof(tight_buffer), "tag", "this_is_a_very_long_value");
    if (res == -2 && strcmp(tight_buffer, original_xml_content) == 0) tests_passed++; else { printf("FAIL: set_tag_string Overflow TC1. Got %d\n", res); assert(0); }
    strcpy(tight_buffer, "<tag>val</tag>"); tests_run++;
    res = set_tag_string(tight_buffer, sizeof(tight_buffer), "tag", "value_fits_tight");
    if (res == XML_UTIL_SUCCESS && strcmp(tight_buffer, "<tag>value_fits_tight</tag>") == 0) tests_passed++; else { printf("FAIL: set_tag_string Overflow TC2. Got %d\n", res); assert(0); }
    char tiny_buf[10];
    strcpy(tiny_buf, "<t>v</t>"); strcpy(original_xml_content, tiny_buf); tests_run++;
    res = set_tag_string(tiny_buf, sizeof(tiny_buf), "t", "123");
    if (res == -2 && strcmp(tiny_buf, original_xml_content) == 0) tests_passed++; else { printf("FAIL: set_tag_string Overflow TC3. Got %d\n", res); assert(0); }
    char eleven_buf[11];
    strcpy(eleven_buf, "<t>v</t>"); tests_run++;
    res = set_tag_string(eleven_buf, sizeof(eleven_buf), "t", "123");
    if (res == XML_UTIL_SUCCESS && strcmp(eleven_buf, "<t>123</t>") == 0) tests_passed++; else { printf("FAIL: set_tag_string Exact Fit TC. Got %d\n", res); assert(0); }
    strcpy(tiny_buf, "<t>v</t>"); strcpy(original_xml_content, tiny_buf); tests_run++;
    res = set_tag_string(tiny_buf, sizeof(tiny_buf), "t", "1234");
    if (res == -2 && strcmp(tiny_buf, original_xml_content) == 0) tests_passed++; else { printf("FAIL: set_tag_string Overflow TC4. Got %d\n", res); assert(0); }


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
        "  <ghz_country_code type=\"string\" length=\"4\" access=\"510\">Q2</ghz_country_code>\n" // Corrected tag name
        "  <ghz_country_code_revision type=\"unsignedByte\" length=\"1\" access=\"510\">7</ghz_country_code_revision>\n" // Corrected tag name
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
        "  <ghz_country_code type=\"string\" length=\"4\" access=\"510\">Q2</ghz_country_code>\n" // Corrected tag name
        "  <ghz_country_code_revision type=\"unsignedByte\" length=\"1\" access=\"510\">7</ghz_country_code_revision>\n" // Corrected tag name
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
    printf("Starting XML helper tests...\n"); // Changed from "sysprod XML helper tests"
    int all_suites_passed = 1;

    if (!test_find_tag_value_start()) {
        all_suites_passed = 0;
        printf("FAIL: test_find_tag_value_start suite failed.\n");
    }
    if (!test_find_closing_tag_pos()) { // Added call to new test suite
        all_suites_passed = 0;
        printf("FAIL: test_find_closing_tag_pos suite failed.\n");
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
        printf("\nAll XML helper tests PASSED.\n"); // Changed from "sysprod XML helper tests"
        return 0; // Success
    } else {
        printf("\nOne or more XML helper tests FAILED.\n"); // Changed
        return 1; // Failure
    }
}
