package qqq6.prj66.asset.assetsmanagement.request.receipt;

import kd.bos.dataentity.OperateOption;
import kd.bos.dataentity.entity.DynamicObject;
import kd.bos.dataentity.entity.DynamicObjectCollection;
import kd.bos.dataentity.metadata.dynamicobject.DynamicObjectType;
import kd.bos.entity.plugin.AbstractOperationServicePlugIn;
import kd.bos.entity.plugin.PreparePropertysEventArgs;
import kd.bos.entity.plugin.args.BeginOperationTransactionArgs;
import kd.bos.orm.query.QCP;
import kd.bos.orm.query.QFilter;
import kd.bos.servicehelper.BusinessDataServiceHelper;
import kd.bos.servicehelper.operation.SaveServiceHelper;

import java.math.BigDecimal;
import java.util.*;

public class AuditPlugin extends AbstractOperationServicePlugIn {

    private static final String KEY_RECEIPTENTRY_ENTRY = "qqq6_receiptentry";
    private static final String KEY_ASSETSNUM = "qqq6_assetsnum";
    private static final String KEY_ASSIGNQTY = "qqq6_assignqty";

    private static final String KEY_ASSETSCARD_BASE = "qqq6_assetscard";
    private static final String KEY_USABLEQTY = "qqq6_usableqty";
    private static final String KEY_ISSTORED = "qqq6_isstored";


    @Override
    public void onPreparePropertys(PreparePropertysEventArgs e) {
        super.onPreparePropertys(e);
        e.getFieldKeys().add(KEY_ASSETSNUM);
        e.getFieldKeys().add(KEY_ASSIGNQTY);
    }

    @Override
    public void beginOperationTransaction(BeginOperationTransactionArgs e) {
        super.beginOperationTransaction(e);
        // 修改资产数量
        changeAssetsQty(e.getDataEntities());
    }

    private void changeAssetsQty(DynamicObject[] receiptBills) {

        // 变化的资产map
        Map<String, BigDecimal> qtyMap = new HashMap<>();
        qtyMap.keySet().toArray(new String[0]);

        for (DynamicObject receiptBill : receiptBills) {
            DynamicObjectCollection receiptEntry = receiptBill.getDynamicObjectCollection(KEY_RECEIPTENTRY_ENTRY);
//            DynamicObjectType dynamicObjectType = receiptEntry.getDynamicObjectType();
            // 领用单数据
            for (DynamicObject row : receiptEntry) {
                DynamicObject asset = row.getDynamicObject(KEY_ASSETSNUM);
                String assetsNum = asset.getString("number");
                BigDecimal qty = row.getBigDecimal(KEY_ASSIGNQTY);
                qtyMap.put(assetsNum, qty);
            }
        }


        // 过滤条件和查询字段
        QFilter[] filters = {new QFilter("number", QCP.in, qtyMap.keySet().toArray(new String[0]))};
        String fields ="id, number, " + KEY_ISSTORED + "," + KEY_USABLEQTY;

        // 资产集合
        DynamicObject[] assets = BusinessDataServiceHelper.load(KEY_ASSETSCARD_BASE, fields, filters);

        // 逐资产修改
        for (DynamicObject asset : assets) {
            BigDecimal newQty = asset.getBigDecimal(KEY_USABLEQTY).subtract(qtyMap.get(asset.getString("number")));
            asset.set(KEY_USABLEQTY, newQty);
            if (newQty.compareTo(BigDecimal.ZERO) == 0) {
                asset.set(KEY_ISSTORED, false);
            } else {
                asset.set(KEY_ISSTORED, true);
            }
        }

        // 保存修改
        SaveServiceHelper.saveOperate(KEY_ASSETSCARD_BASE, assets, OperateOption.create());
    }
}
