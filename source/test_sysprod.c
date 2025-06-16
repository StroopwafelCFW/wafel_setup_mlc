#include <stdio.h>
#include <string.h>
#include <assert.h> // For assert()

// For the functions we are testing:
// find_tag_value_start, get_tag_string, set_tag_string
// These are declared in sysprod.h.
// We assume that when compiling these tests, the include paths are set up
// correctly to find "sysprod.h" and its dependencies like "wafel/ios/svc.h".
#include "sysprod.h"

// Test suite for find_tag_value_start
static int test_find_tag_value_start() {
    printf("Running tests for find_tag_value_start...\n");
    int tests_passed = 0;
    int tests_run = 0;

    // Test case 1: Simple tag
    const char* xml1 = "<test>value</test>";
    tests_run++;
    const char* result1 = find_tag_value_start(xml1, "test");
    assert(result1 != NULL && strcmp(result1, "value</test>") == 0);
    if (result1 != NULL && strcmp(result1, "value</test>") == 0) tests_passed++;

    // Test case 2: Nested tag
    const char* xml2 = "<outer><test>val</test></outer>";
    tests_run++;
    const char* result2 = find_tag_value_start(xml2, "test");
    assert(result2 != NULL && strcmp(result2, "val</test></outer>") == 0);
    if (result2 != NULL && strcmp(result2, "val</test></outer>") == 0) tests_passed++;

    // Test case 3: Empty tag
    const char* xml3 = "<test></test>";
    tests_run++;
    const char* result3 = find_tag_value_start(xml3, "test");
    assert(result3 != NULL && strcmp(result3, "</test>") == 0);
    if (result3 != NULL && strcmp(result3, "</test>") == 0) tests_passed++;

    // Test case 4: Tag among others
    const char* xml4 = "<anothertag>value</anothertag><test>value2</test>";
    tests_run++;
    const char* result4 = find_tag_value_start(xml4, "test");
    assert(result4 != NULL && strcmp(result4, "value2</test>") == 0);
    if (result4 != NULL && strcmp(result4, "value2</test>") == 0) tests_passed++;

    // Test case 5: Non-existent tag
    tests_run++;
    const char* result5 = find_tag_value_start(xml1, "nonexistent");
    assert(result5 == NULL);
    if (result5 == NULL) tests_passed++;

    // Test case 6: Null XML content
    tests_run++;
    const char* result6 = find_tag_value_start(NULL, "test");
    assert(result6 == NULL);
    if (result6 == NULL) tests_passed++;

    // Test case 7: Null tag name
    tests_run++;
    const char* result7 = find_tag_value_start(xml1, NULL);
    assert(result7 == NULL);
    if (result7 == NULL) tests_passed++;

    printf("find_tag_value_start tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}

// Test suite for get_tag_string
static int test_get_tag_string() {
    printf("Running tests for get_tag_string...\n");
    char buffer[50];
    int tests_passed = 0;
    int tests_run = 0;

    // Test case 1: Simple extraction
    const char* xml1 = "<test>value</test>";
    tests_run++;
    assert(get_tag_string(xml1, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "value") == 0);
    if (get_tag_string(xml1, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "value") == 0) tests_passed++;

    // Test case 2: Nested tag extraction
    const char* xml2 = "<outer><test>val</test></outer>";
    tests_run++;
    assert(get_tag_string(xml2, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "val") == 0);
    if (get_tag_string(xml2, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "val") == 0) tests_passed++;

    // Test case 3: Empty value tag
    const char* xml3 = "<test></test>";
    tests_run++;
    assert(get_tag_string(xml3, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "") == 0);
    if (get_tag_string(xml3, "test", buffer, sizeof(buffer)) != NULL && strcmp(buffer, "") == 0) tests_passed++;

    // Test case 4: Buffer truncation
    const char* xml4 = "<test>longvalue12345</test>";
    tests_run++;
    char small_buffer[10];
    assert(get_tag_string(xml4, "test", small_buffer, sizeof(small_buffer)) != NULL && strcmp(small_buffer, "longvalue") == 0 && strlen(small_buffer) == 9);
    if (get_tag_string(xml4, "test", small_buffer, sizeof(small_buffer)) != NULL && strcmp(small_buffer, "longvalue") == 0 && strlen(small_buffer) == 9) tests_passed++;

    // Test case 5: Non-existent tag
    tests_run++;
    assert(get_tag_string(xml1, "nonexistent", buffer, sizeof(buffer)) == NULL);
    if (get_tag_string(xml1, "nonexistent", buffer, sizeof(buffer)) == NULL) tests_passed++;

    // Test case 6: Tag exists but no closing tag
    const char* xml5 = "<test>value_no_close";
    tests_run++;
    assert(get_tag_string(xml5, "test", buffer, sizeof(buffer)) == NULL);
    if (get_tag_string(xml5, "test", buffer, sizeof(buffer)) == NULL) tests_passed++;

    // Test case 7: Buffer size too small (size 1, only for null terminator)
    const char* xml6 = "<test>a</test>";
    tests_run++;
    char tiny_buffer[1];
    assert(get_tag_string(xml6, "test", tiny_buffer, sizeof(tiny_buffer)) != NULL && strcmp(tiny_buffer, "") == 0);
    if (get_tag_string(xml6, "test", tiny_buffer, sizeof(tiny_buffer)) != NULL && strcmp(tiny_buffer, "") == 0) tests_passed++;

    printf("get_tag_string tests: %d/%d PASSED\n", tests_passed, tests_run);
    return tests_passed == tests_run;
}

// Test suite for set_tag_string
static int test_set_tag_string() {
    printf("Running tests for set_tag_string...\n");
    char xml_buffer[256]; // Mutable buffer for testing
    int tests_passed = 0;
    int tests_run = 0;

    // Test case 1: New value same length
    tests_run++;
    strcpy(xml_buffer, "<root><item>old</item></root>");
    assert(set_tag_string(xml_buffer, "item", "new") == 0 && strcmp(xml_buffer, "<root><item>new</item></root>") == 0);
    if (set_tag_string(xml_buffer, "item", "new") == 0 && strcmp(xml_buffer, "<root><item>new</item></root>") == 0) tests_passed++;

    // Test case 2: New value shorter
    tests_run++;
    strcpy(xml_buffer, "<root><item>longer</item></root>");
    assert(set_tag_string(xml_buffer, "item", "short") == 0 && strcmp(xml_buffer, "<root><item>short</item></root>") == 0);
    if (set_tag_string(xml_buffer, "item", "short") == 0 && strcmp(xml_buffer, "<root><item>short</item></root>") == 0) tests_passed++;

    // Test case 3: New value longer
    tests_run++;
    strcpy(xml_buffer, "<root><item>short</item></root>");
    assert(set_tag_string(xml_buffer, "item", "much_longer_value") == 0 && strcmp(xml_buffer, "<root><item>much_longer_value</item></root>") == 0);
    if (set_tag_string(xml_buffer, "item", "much_longer_value") == 0 && strcmp(xml_buffer, "<root><item>much_longer_value</item></root>") == 0) tests_passed++;

    // Test case 4: Empty new value
    tests_run++;
    strcpy(xml_buffer, "<root><item>data</item></root>");
    assert(set_tag_string(xml_buffer, "item", "") == 0 && strcmp(xml_buffer, "<root><item></item></root>") == 0);
    if (set_tag_string(xml_buffer, "item", "") == 0 && strcmp(xml_buffer, "<root><item></item></root>") == 0) tests_passed++;

    // Test case 5: Setting value for an initially empty tag
    tests_run++;
    strcpy(xml_buffer, "<root><item></item></root>");
    assert(set_tag_string(xml_buffer, "item", "filled") == 0 && strcmp(xml_buffer, "<root><item>filled</item></root>") == 0);
    if (set_tag_string(xml_buffer, "item", "filled") == 0 && strcmp(xml_buffer, "<root><item>filled</item></root>") == 0) tests_passed++;

    // Test case 6: Tag not found
    tests_run++;
    strcpy(xml_buffer, "<root><item>value</item></root>");
    char original_xml[256]; // Store original to check it's unchanged
    strcpy(original_xml, xml_buffer);
    assert(set_tag_string(xml_buffer, "nonexistent", "new") == -1 && strcmp(xml_buffer, original_xml) == 0);
    if (set_tag_string(xml_buffer, "nonexistent", "new") == -1 && strcmp(xml_buffer, original_xml) == 0) tests_passed++;

    // Test case 7: Multiple tags, ensure only correct one is changed
    tests_run++;
    strcpy(xml_buffer, "<data><val1>abc</val1><val2>def</val2></data>");
    assert(set_tag_string(xml_buffer, "val1", "xyz") == 0 && strcmp(xml_buffer, "<data><val1>xyz</val1><val2>def</val2></data>") == 0);
    // Second modification on the same buffer
    assert(set_tag_string(xml_buffer, "val2", "jkl") == 0 && strcmp(xml_buffer, "<data><val1>xyz</val1><val2>jkl</val2></data>") == 0);
    // Check state after both modifications
    if (strcmp(xml_buffer, "<data><val1>xyz</val1><val2>jkl</val2></data>") == 0) tests_passed++;


    // Test case 8: Tag at the end of the document
    tests_run++;
    strcpy(xml_buffer, "<first>one</first><second>two</second>");
    assert(set_tag_string(xml_buffer, "second", "new_two") == 0 && strcmp(xml_buffer, "<first>one</first><second>new_two</second>") == 0);
    if (set_tag_string(xml_buffer, "second", "new_two") == 0 && strcmp(xml_buffer, "<first>one</first><second>new_two</second>") == 0) tests_passed++;

    printf("set_tag_string tests: %d/%d PASSED\n", tests_passed, tests_run);
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

    if (all_suites_passed) {
        printf("\nAll sysprod XML helper tests PASSED.\n");
        return 0; // Success
    } else {
        printf("\nOne or more sysprod XML helper tests FAILED.\n");
        return 1; // Failure
    }
}
