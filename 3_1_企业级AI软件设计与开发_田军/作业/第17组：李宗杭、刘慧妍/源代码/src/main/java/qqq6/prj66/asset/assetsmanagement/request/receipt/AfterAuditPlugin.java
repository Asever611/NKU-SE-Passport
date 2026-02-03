package qqq6.prj66.asset.assetsmanagement.request.receipt;

import kd.bos.dataentity.OperateOption;
import kd.bos.dataentity.entity.DynamicObject;
import kd.bos.dataentity.entity.DynamicObjectCollection;
import kd.bos.form.control.Toolbar;
import kd.bos.form.control.events.ItemClickEvent;
import kd.bos.form.plugin.AbstractFormPlugin;
import kd.bos.orm.query.QCP;
import kd.bos.orm.query.QFilter;
import kd.bos.servicehelper.BusinessDataServiceHelper;
import kd.bos.servicehelper.operation.SaveServiceHelper;

import java.math.BigDecimal;
import java.util.*;

public class AfterAuditPlugin extends AbstractFormPlugin {

    private static final String KEY_RECEIPTENTRY_ENTRY = "qqq6_receiptentry";
    private static final String KEY_ASSETSNUM = "qqq6_assetsnum";
    private static final String KEY_ASSIGNQTY = "qqq6_assignqty";

    private static final String KEY_ASSETSCARD_BASE = "qqq6_assetscard";
    private static final String KEY_USABLEQTY = "qqq6_usableqty";


    @Override
    public void registerListener(EventObject e) {
        super.registerListener(e);
        // 监听工具栏按钮点击
        Toolbar toolbar = this.getView().getControl("tbmain");
        if (toolbar != null) {
            toolbar.addItemClickListener(this);
        }
    }


    @Override
    public void itemClick(ItemClickEvent evt) {
        super.itemClick(evt);
        if ("bar_audit".equals(evt.getItemKey())) {
            // 修改资产数量
            changeAssetsQty();
        }
    }

    private void changeAssetsQty(){

        // 变化的资产map
        Map<String, BigDecimal> qtyMap = new HashMap<>();
        qtyMap.keySet().toArray(new String[0]);

        // 领用单数据
        DynamicObjectCollection receiptEntry = this.getModel().getDataEntity().getDynamicObjectCollection(KEY_RECEIPTENTRY_ENTRY);
        for (DynamicObject row : receiptEntry) {
            String assetsNum = row.getString(KEY_ASSETSNUM + ".number");
            BigDecimal qty = row.getBigDecimal(KEY_ASSIGNQTY);
            qtyMap.put(assetsNum, qty);
        }

        // 过滤条件和查询字段
        QFilter[] filters = {new QFilter("number", QCP.in, qtyMap.keySet().toArray(new String[0]))};
        String fields ="id, number, " + KEY_USABLEQTY;

        // 资产集合
        DynamicObject[] assets = BusinessDataServiceHelper.load(KEY_ASSETSCARD_BASE, fields, filters);

        // 逐资产修改
        for (DynamicObject asset : assets) {
            BigDecimal newQty = asset.getBigDecimal(KEY_USABLEQTY).subtract(qtyMap.get(asset.getString("number")));
            asset.set(KEY_USABLEQTY, newQty);
        }

        // 保存修改
        SaveServiceHelper.saveOperate(KEY_ASSETSCARD_BASE, assets, OperateOption.create());
    }
}
