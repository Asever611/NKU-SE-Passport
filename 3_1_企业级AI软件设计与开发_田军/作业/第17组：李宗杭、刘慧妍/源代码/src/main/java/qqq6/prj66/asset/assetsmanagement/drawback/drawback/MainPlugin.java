package qqq6.prj66.asset.assetsmanagement.drawback.drawback;

import kd.bos.bill.AbstractBillPlugIn;
import kd.bos.dataentity.entity.DynamicObject;
import kd.bos.dataentity.entity.DynamicObjectCollection;
import kd.bos.dataentity.metadata.IDataEntityProperty;
import kd.bos.entity.datamodel.AbstractFormDataModel;
import kd.bos.entity.datamodel.TableValueSetter;
import kd.bos.entity.datamodel.events.PropertyChangedArgs;
import kd.bos.orm.query.QCP;
import kd.bos.orm.query.QFilter;
import kd.bos.servicehelper.QueryServiceHelper;

import java.math.BigDecimal;
import java.util.*;

/**
 * 资产盘点单插件
 */
public class MainPlugin extends AbstractBillPlugIn {

    // 资产退库单
    private static final String KEY_RETURNENTRY_ENTRY = "qqq6_returnentry"; // 单据体
    private static final String KEY_REQUESTER = "qqq6_requester"; // 申请人
    private static final String KEY_ASSETSNUM = "qqq6_assetsnum"; // 资产编号
    private static final String KEY_ORIGINALPICTURE = "qqq6_originalpicture"; // 原资产图片
    private static final String KEY_QTY = "qqq6_qty"; // 资产数量

    // 资产领用单
    private static final String KEY_RECEIPTBILL_BILL = "qqq6_receiptbill"; // 单据
    private static final String KEY_RECEIPTENTRY_ENTRY = "qqq6_receiptentry"; // 单据体
    private static final String KEY_ASSIGNQTY = "qqq6_assignqty"; // 分配数量

    // 资产卡片
    private static final String KEY_ASSETSCARD_BASE = "qqq6_assetscard";
    private static final String KEY_PICTURE = "qqq6_picture"; // 资产图片
    private static final String KEY_UNIT = "qqq6_unit";

    @Override
    public void propertyChanged(PropertyChangedArgs e) {
        IDataEntityProperty property = e.getProperty();
        if (property.getName().equals(KEY_REQUESTER)) {
            // 若发生改变的字段是申请人，执行该快捷方法
            setRows();
        }
        super.propertyChanged(e);
    }

    /**
     * 快捷方法
     */
    private void setRows(){
        DynamicObject requester = (DynamicObject) this.getModel().getValue(KEY_REQUESTER); // 获取申请人

        AbstractFormDataModel model =  (AbstractFormDataModel) this.getModel(); // 表单数据模型
        model.deleteEntryData(KEY_RETURNENTRY_ENTRY); // 每次调用该方法先清除原本显示的所有行

        if (requester == null){ // 若未选择方案
            this.getView().updateView();
            return;
        }

        // 执行添加行方法
        addRows(getAssetsByRequester(requester));

        this.getView().updateView();
    }

    /**
     * 批量新增分录行
     */
    private void addRows(HashMap<Long, BigDecimal> assets){

        AbstractFormDataModel model =  (AbstractFormDataModel) this.getModel();

        // 开启事务
        model.beginInit();
        // 设置字段
        TableValueSetter setter = new TableValueSetter();
        setter.addField(KEY_ASSETSNUM);
        setter.addField(KEY_QTY);
        setter.addField(KEY_UNIT);
        setter.addField(KEY_ORIGINALPICTURE);
        // 添加数据
        for (Long asset : assets.keySet()) {

            QFilter[] filters = {new QFilter("id", QCP.equals, asset)};
            String fields = "id, " + KEY_UNIT + ".id, " + KEY_PICTURE;
            DynamicObjectCollection dynamicObjects = QueryServiceHelper.query(KEY_ASSETSCARD_BASE, fields, filters);
            long unitId = dynamicObjects.get(0).getLong(KEY_UNIT + ".id");
            String picture = dynamicObjects.get(0).getString(KEY_PICTURE);
            setter.addRow(
                    asset,
                    assets.get(asset),
                    unitId,
                    picture
                );

        }
        // 批量新增
        model.batchCreateNewEntryRow(KEY_RETURNENTRY_ENTRY, setter);
        model.endInit();
    }

    /**
     * 获取资产
     */
    private HashMap<Long, BigDecimal> getAssetsByRequester(DynamicObject requester) {
        // 过滤条件
        long id = requester.getLong("id"); // 获取申请人id
        QFilter[] filter = {new QFilter(KEY_REQUESTER, QCP.equals, id)};

        // 字段
        StringBuilder fields = new StringBuilder();
        fields.append(KEY_RECEIPTENTRY_ENTRY).append(".").append(KEY_ASSETSNUM).append(".id").append(","); // 资产编号
        fields.append(KEY_RECEIPTENTRY_ENTRY).append(".").append(KEY_ASSIGNQTY); // 数量

        DynamicObjectCollection dynamicObjects = QueryServiceHelper.query(KEY_RECEIPTBILL_BILL, fields.toString(), filter);

        HashMap<Long, BigDecimal> assets = new HashMap<>();
        // 查询该申请人所有领用单的所有资产num
        for (DynamicObject dynamicObject : dynamicObjects) {
            assets.put(
                dynamicObject.getLong(KEY_RECEIPTENTRY_ENTRY + "." + KEY_ASSETSNUM + ".id"),
                dynamicObject.getBigDecimal(KEY_RECEIPTENTRY_ENTRY + "." + KEY_ASSIGNQTY)
            );
        }
        return assets;
    }
}
