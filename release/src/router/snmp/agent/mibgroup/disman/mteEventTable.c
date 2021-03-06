/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.create-dataset.conf,v 5.2 2002/07/17 14:41:53 dts12 Exp $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "mteTriggerTable.h"
#include "mteEventTable.h"
#include "mteEventNotificationTable.h"
#include "mteObjectsTable.h"

netsnmp_table_data_set *table_set = NULL;

/** Initialize the mteEventTable table by defining its contents and how it's structured */
void
initialize_table_mteEventTable(void)
{
    static oid      mteEventTable_oid[] =
        { 1, 3, 6, 1, 2, 1, 88, 1, 4, 2 };
    size_t          mteEventTable_oid_len = OID_LENGTH(mteEventTable_oid);

    /*
     * create the table structure itself 
     */
    table_set = netsnmp_create_table_data_set("mteEventTable");

    /*
     * comment this out or delete if you don't support creation of new rows 
     */
    table_set->allow_creation = 1;
    /* mark the row status column */
    table_set->rowstatus_column = COLUMN_MTEEVENTENTRYSTATUS;

    /***************************************************
     * Adding indexes
     */
    DEBUGMSGTL(("initialize_table_mteEventTable",
                "adding indexes to table mteEventTable\n"));
    netsnmp_table_set_add_indexes(table_set,
                                  ASN_OCTET_STR,   /* index: mteOwner */
                                  ASN_PRIV_IMPLIED_OCTET_STR, /* index: mteEventName */
                                  0);

    DEBUGMSGTL(("initialize_table_mteEventTable",
                "adding column types to table mteEventTable\n"));
    netsnmp_table_set_multi_add_default_row(table_set,
                                            COLUMN_MTEEVENTNAME,
                                            ASN_OCTET_STR, 0, NULL, 0,
                                            COLUMN_MTEEVENTCOMMENT,
                                            ASN_OCTET_STR, 1, NULL, 0,
                                            COLUMN_MTEEVENTACTIONS,
                                            ASN_OCTET_STR, 1, NULL, 0,
                                            COLUMN_MTEEVENTENABLED,
                                            ASN_INTEGER, 1, NULL, 0,
                                            COLUMN_MTEEVENTENTRYSTATUS,
                                            ASN_INTEGER, 1, NULL, 0, 0);

    /* keep index values around for comparisons later */
    table_set->table->store_indexes = 1;
    /*
     * registering the table with the master agent 
     */
    /*
     * note: if you don't need a subhandler to deal with any aspects
     * of the request, change mteEventTable_handler to "NULL" 
     */
    netsnmp_register_table_data_set(netsnmp_create_handler_registration
                                    ("mteEventTable",
                                     mteEventTable_handler,
                                     mteEventTable_oid,
                                     mteEventTable_oid_len,
                                     HANDLER_CAN_RWRITE), table_set, NULL);
}

/** Initializes the mteEventTable module */
void
init_mteEventTable(void)
{

    /*
     * here we initialize all the tables we're planning on supporting 
     */
    initialize_table_mteEventTable();
}

/** handles requests for the mteEventTable table, if anything else needs to be done */
int
mteEventTable_handler(netsnmp_mib_handler *handler,
                      netsnmp_handler_registration *reginfo,
                      netsnmp_agent_request_info *reqinfo,
                      netsnmp_request_info *requests)
{
    /*
     * perform anything here that you need to do.  The requests have
     * already been processed by the master table_dataset handler, but
     * this gives you chance to act on the request in some other way
     * if need be. 
     */

    /* XXX: on rowstatus = destroy, remove the corresponding rows from the
       other tables: snmpEventNotificationTable and the set table */
    return SNMP_ERR_NOERROR;
}

/*
 * send trap 
 */
void
run_mte_events(struct mteTriggerTable_data *item,
               oid * name_oid, size_t name_oid_len,
               const char *eventobjowner, const char *eventobjname)
{
    static oid      objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };     /* snmpTrapIOD.0 */

    netsnmp_variable_list *var_list = NULL;

    netsnmp_table_row *row, *notif_row;
    netsnmp_table_data_set_storage *col1, *tc, *no, *noo;

    for(row = table_set->table->first_row; row; row = row->next) {
        if (strcmp(row->indexes->val.string, eventobjowner) == 0 &&
            strcmp(row->indexes->next_variable->val.string,
                   eventobjname) == 0) {
            /* run this event */
            col1 = (netsnmp_table_data_set_storage *) row->data;
            
            tc = netsnmp_table_data_set_find_column(col1,
                                                    COLUMN_MTEEVENTACTIONS);
            if (!tc->data.bitstring[0] & 0x80) {
                /* not a notification.  next! (XXX: do sets) */
                continue;
            }

            tc = netsnmp_table_data_set_find_column(col1,
                                                    COLUMN_MTEEVENTENABLED);
            if (*(tc->data.integer) != 1) {
                /* not enabled.  next! */
                continue;
            }

            if (!mteEventNotif_table_set) {
                /* no notification info */
                continue;
            }

            /* send the notification */
            var_list = NULL;

            /* XXX: get notif information */
            for(notif_row = mteEventNotif_table_set->table->first_row;
                notif_row; notif_row = notif_row->next) {
                if (strcmp(notif_row->indexes->val.string,
                           eventobjowner) == 0 &&
                    strcmp(notif_row->indexes->next_variable->val.string,
                           eventobjname) == 0) {

                    /* run this event */
                    col1 = (netsnmp_table_data_set_storage *) notif_row->data;
            
                    tc = netsnmp_table_data_set_find_column(col1, COLUMN_MTEEVENTNOTIFICATION);
                    no = netsnmp_table_data_set_find_column(col1, COLUMN_MTEEVENTNOTIFICATIONOBJECTS);
                    noo = netsnmp_table_data_set_find_column(col1, COLUMN_MTEEVENTNOTIFICATIONOBJECTSOWNER);
                    if (!tc)
                        continue; /* no notification to be had. XXX: return? */
                    
                    /*
                     * snmpTrap oid 
                     */
                    snmp_varlist_add_variable(&var_list, objid_snmptrap,
                                              sizeof(objid_snmptrap) /
                                              sizeof(oid),
                                              ASN_OBJECT_ID,
                                              (u_char *) tc->data.objid,
                                              tc->data_len);

                    /* XXX: add objects from the mteObjectsTable */
                    if (no && no->data.string &&
                        noo && noo->data.string && name_oid) {
                        mte_add_objects(var_list, item,
                                        noo->data.string,
                                        no->data.string,
                                        name_oid + item->mteTriggerValueIDLen,
                                        name_oid_len - item->mteTriggerValueIDLen);
                    }

                    DEBUGMSGTL(("mteEventTable:send_events", "sending an event "));
                    DEBUGMSGOID(("mteEventTable:send_events", tc->data.objid, tc->data_len / sizeof(oid)));
                    DEBUGMSG(("mteEventTable:send_events", "\n"));
                    
                    send_v2trap(var_list);
                    snmp_free_varbind(var_list);
                }
            }
        }
    }
}
