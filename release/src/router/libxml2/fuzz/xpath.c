/*
 * xpath.c: a libFuzzer target to test XPath and XPointer expressions.
 *
 * See Copyright for the status of this software.
 */

#include <libxml/parser.h>
#include <libxml/xpointer.h>
#include "fuzz.h"

int
LLVMFuzzerInitialize(int *argc ATTRIBUTE_UNUSED,
                     char ***argv ATTRIBUTE_UNUSED) {
    xmlInitParser();
    xmlSetGenericErrorFunc(NULL, xmlFuzzErrorFunc);

    return 0;
}

int
LLVMFuzzerTestOneInput(const char *data, size_t size) {
    xmlDocPtr doc;
    const char *expr, *xml;
    size_t exprSize, xmlSize;

    if (size > 10000)
        return(0);

    xmlFuzzDataInit(data, size);

    expr = xmlFuzzReadString(&exprSize);
    xml = xmlFuzzReadString(&xmlSize);

    /* Recovery mode allows more input to be fuzzed. */
    doc = xmlReadMemory(xml, xmlSize, NULL, NULL, XML_PARSE_RECOVER);
    if (doc != NULL) {
        xmlXPathContextPtr xpctxt = xmlXPathNewContext(doc);

        /* Operation limit to avoid timeout */
        xpctxt->opLimit = 500000;

        xmlXPathFreeObject(xmlXPtrEval(BAD_CAST expr, xpctxt));
        xmlXPathFreeContext(xpctxt);
    }
    xmlFreeDoc(doc);

    xmlFuzzDataCleanup();
    xmlResetLastError();

    return(0);
}

