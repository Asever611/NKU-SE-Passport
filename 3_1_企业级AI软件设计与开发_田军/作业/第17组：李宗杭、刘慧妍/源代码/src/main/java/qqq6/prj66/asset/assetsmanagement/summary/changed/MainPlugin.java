package qqq6.prj66.asset.assetsmanagement.summary.changed;

import kd.bos.algo.DataSet;
import kd.bos.context.RequestContext;
import kd.bos.dataentity.entity.DynamicObject;
import kd.bos.db.DB;
import kd.bos.db.DBRoute;
import kd.bos.entity.report.AbstractReportListDataPlugin;
import kd.bos.entity.report.FilterItemInfo;
import kd.bos.entity.report.ReportQueryParam;
import kd.bos.servicehelper.user.UserServiceHelper;

import java.text.SimpleDateFormat;
import java.util.Date;

public class MainPlugin extends AbstractReportListDataPlugin {

    @Override
    public DataSet query(ReportQueryParam reportQueryParam, Object o) throws Throwable {

        StringBuilder filters = new StringBuilder();
        for(FilterItemInfo filter : reportQueryParam.getFilter().getFilterItems()){
            String prop = filter.getPropName();

            if (filter.getValue() == null) continue;
            if (filters.length() > 0) filters.append(" and ");
            if (prop.equals("qqq6_assetsorg_f")){
                DynamicObject value = (DynamicObject) filter.getValue();
                String orgId = value.getString("id");
                filters.append("bill.fk_qqq6_reqorg").append(" = ").append(orgId);
            }
            if (prop.equals("qqq6_assetsname_f")){
                DynamicObject value = (DynamicObject) filter.getValue();
                String assetsId = value.getString("id");
                filters.append("entry.fk_qqq6_assetsnum").append(" = ").append(assetsId);
            }
            if (prop.equals("qqq6_changeddate_f")){
                Date value = (Date) filter.getValue();
                String date = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(value);
                filters.append("bill.fk_qqq6_reqdate").append(" = '").append(date).append("'");
            }
            if (prop.equals("qqq6_user_f")){
                DynamicObject value = (DynamicObject) filter.getValue();
                String userId = value.getString("id");
                filters.append("bill.fk_qqq6_requester").append(" = ").append(userId);
            }
        }

        String algoKey = this.getClass().getName();
        DBRoute dbRoute = DBRoute.of("secd");

        String currId = String.valueOf(UserServiceHelper.getCurrentUserId());

        String reqSQL = "select " +
                "entry.fk_qqq6_assetsnum as qqq6_assetsnum, " +
                "entry.fk_qqq6_unit as qqq6_unit, " +
                "entry.fk_qqq6_assignqty as qqq6_changedqty, " +
                "entry.fk_qqq6_remark as qqq6_remark, " +
                "'0' as qqq6_changedtype, " +
                "bill.fk_qqq6_reqdate as qqq6_changeddate, " +
                "bill.fk_qqq6_requester as qqq6_user " +
                "from tk_qqq6_receiptbill bill left join tk_qqq6_receiptentry entry " +
                "on bill.fid=entry.fid " +
                "left join tk_qqq6_assetscard assets " +
                "on entry.fk_qqq6_assetsnum=assets.fid " +
                "where fk_qqq6_requester=" + currId;

        String returnSQL = "select " +
                "entry.fk_qqq6_assetsnum as qqq6_assetsnum, " +
                "entry.fk_qqq6_unit as qqq6_unit, " +
                "entry.fk_qqq6_qty as qqq6_changedqty, " +
                "entry.fk_qqq6_remark as qqq6_remark, " +
                "'1' as qqq6_changedtype, " +
                "bill.fk_qqq6_reqdate as qqq6_changeddate, " +
                "bill.fk_qqq6_requester as qqq6_user " +
                "from tk_qqq6_returnbill bill left join tk_qqq6_returnentry entry " +
                "on bill.fid=entry.fid " +
                "left join tk_qqq6_assetscard assets " +
                "on entry.fk_qqq6_assetsnum=assets.fid " +
                "where fk_qqq6_requester=" + currId;

        if (!filters.toString().isEmpty()){
            reqSQL += " and " + filters.toString();
            returnSQL += " and " + filters.toString();
        }

        DataSet reqDataSet = DB.queryDataSet(algoKey, dbRoute, reqSQL);
        DataSet returnDataSet = DB.queryDataSet(algoKey, dbRoute, returnSQL);
        return reqDataSet.union(returnDataSet);
    }

}
