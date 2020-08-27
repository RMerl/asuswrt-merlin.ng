#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<libxml/xmlmemory.h>
#include<libxml/parser.h>

int main()
{
 xmlDocPtr doc=xmlNewDoc(BAD_CAST"1.0");
 xmlNodePtr root_node=xmlNewNode(NULL,BAD_CAST"root");
 xmlDocSetRootElement(doc,root_node);
 xmlNewTextChild(root_node,NULL,BAD_CAST "newNode1",BAD_CAST "newNode1 content");
 xmlNewTextChild(root_node,NULL,BAD_CAST "newNode2",BAD_CAST "newNode2 content");
 xmlNewTextChild(root_node,NULL,BAD_CAST "newNode3",BAD_CAST "newNode3 content");
 xmlNodePtr node=xmlNewNode(NULL,BAD_CAST "node2");
 xmlNodePtr content=xmlNewText(BAD_CAST "NODE CONTENT");
 xmlAddChild(root_node,node);
 xmlAddChild(node,content);
 xmlNewProp(node,BAD_CAST "attribute",BAD_CAST "yes");
 node=xmlNewNode(NULL,BAD_CAST "son");
 xmlAddChild(root_node,node);
 xmlNodePtr grandson=xmlNewNode(NULL,BAD_CAST "grandson");
 xmlAddChild(node,grandson);
 xmlAddChild(grandson,xmlNewText(BAD_CAST "This is a grandson node"));
 int nRel=xmlSaveFile("CreatedXml.xml",doc);
 if(nRel!=-1)
 {
   xmlFreeDoc(doc);
   return 1;
 }
}
