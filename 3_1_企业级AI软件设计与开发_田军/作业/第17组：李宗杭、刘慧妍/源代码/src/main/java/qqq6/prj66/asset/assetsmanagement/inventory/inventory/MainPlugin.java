package qqq6.prj66.asset.assetsmanagement.inventory.inventory;

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

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;

/**
 * 资产盘点单插件
 */
public class MainPlugin extends AbstractBillPlugIn {

    // 资产盘点单
    private static final String KEY_INVENTORYENTRY_ENTRY = "qqq6_inventoryentry"; // 单据体
    private static final String KEY_PLAN = "qqq6_plan"; // 方案
    private static final String KEY_PLANNAME = "qqq6_planname"; // 方案名称
    private static final String KEY_ASSETSORG = "qqq6_assetsorg"; // 资产组织
    private static final String KEY_ASSETSNUM = "qqq6_assetsnum"; // 资产编号
    private static final String KEY_ASSETSNAME = "qqq6_assetsname"; // 资产名称
    private static final String KEY_BOOKQTY = "qqq6_bookqty"; // 账存数量
    private static final String KEY_FIRSTQTY = "qqq6_firstcountqty"; // 初盘数量
    private static final String KEY_ISDATEVALID = "qqq6_isdatevalid"; // 启用日期合法性

    // 盘点方案
    private static final String KEY_INVENTORYPLAN_BILL = "qqq6_inventoryplan"; // 单据
    private static final String KEY_PLANENTRY_ENTRY = "qqq6_planentry"; // 单据体
    private static final String KEY_BILLNO = "billno"; // 单据编号
    private static final String KEY_ENDDATE = "qqq6_enddate"; // 启用截止日期

    // 资产卡片
    private static final String KEY_ASSETSCARD_BASE = "qqq6_assetscard";
    private static final String KEY_ASSETSQTY = "qqq6_qty"; // 资产数量
    private static final String KEY_UNIT = "qqq6_unit"; // 计量单位

    // 全局变量
    private static boolean isDateValid = true; // 启用截止日期是否合法
    private static String planname = ""; // 方案名称


    @Override
    public void propertyChanged(PropertyChangedArgs e) {
        IDataEntityProperty property = e.getProperty();
        if (property.getName().equals(KEY_PLAN)){
            // 若发生改变的字段是方案，执行该快捷方法
            setRows();
        }
        super.propertyChanged(e);
    }

    /**
     * 快捷方法
     */
    private void setRows(){
        DynamicObject plan = (DynamicObject) this.getModel().getValue(KEY_PLAN); // 获取方案

        AbstractFormDataModel model =  (AbstractFormDataModel) this.getModel(); // 表单数据模型
        model.deleteEntryData(KEY_INVENTORYENTRY_ENTRY); // 每次调用该方法先清除原本显示的所有行

        if (plan == null){ // 若未选择方案
            this.getView().updateView();
            return;
        }

        // 执行添加行方法
        addRows(getAssetsFromOrgs(getOrgsFromPlan(plan)));

        // 设置单据头的方案名称
        this.getModel().setValue(KEY_PLANNAME, planname);
        this.getView().updateView();

        // 在单据头添加了一个隐藏的“启用日期合法性”文本字段用来辅助判断日期合法性
        // 在此更改其值后，通过业务规则来实现功能3
        if (isDateValid){
            this.getModel().setValue(KEY_ISDATEVALID, "true");
        } else {
            this.getModel().setValue(KEY_ISDATEVALID, "false");
        }
    }

    /**
     * 批量新增分录行
     */
    private void addRows(HashMap<String, DynamicObjectCollection> assetsOfOrgs){

        AbstractFormDataModel model =  (AbstractFormDataModel) this.getModel();

        // 开启事务
        model.beginInit();
        // 设置字段
        TableValueSetter setter = new TableValueSetter();
        setter.addField(KEY_ASSETSORG);
        setter.addField(KEY_ASSETSNUM);
        setter.addField(KEY_ASSETSNAME);
        setter.addField(KEY_UNIT);
        setter.addField(KEY_BOOKQTY);
        setter.addField(KEY_FIRSTQTY);
        // 添加数据
        for (DynamicObjectCollection assets : assetsOfOrgs.values()) {
            for (DynamicObject asset : assets) {
                setter.addRow(
                    asset.getLong(KEY_ASSETSORG+".id"),
                    asset.getString("number"),
                    asset.getString("name"),
                    asset.getLong(KEY_UNIT+".id"),
                    asset.getBigDecimal(KEY_ASSETSQTY),
                    asset.getBigDecimal(KEY_ASSETSQTY)
                );
            }
        }
        // 批量新增
        model.batchCreateNewEntryRow(KEY_INVENTORYENTRY_ENTRY, setter);
        model.endInit();
    }

    /**
     * 按组织获取资产
     */
    private HashMap<String, DynamicObjectCollection> getAssetsFromOrgs(List<String> orgs) {
        // 用HashMap没什么意义，用List一样的
        HashMap<String, DynamicObjectCollection> assetsOfOrgs = new HashMap<>();

        StringBuilder fields = new StringBuilder();
        fields.append("number, name,"); // 编号，名称
        fields.append(KEY_ASSETSORG).append(".id, "); // 组织
        fields.append(KEY_UNIT).append(".id, "); // 计量单位
        fields.append(KEY_ASSETSQTY); // 数量

        // 逐组织添加
        for (String org : orgs) {
            QFilter[] filters = {
                new QFilter(KEY_ASSETSORG+".number", QCP.equals, org)
            };
            DynamicObjectCollection assets = QueryServiceHelper.query(KEY_ASSETSCARD_BASE, fields.toString(), filters);
            assetsOfOrgs.put(org, assets);
        }
        return assetsOfOrgs;
    }

    /**
     * 返回资产组织列表
     * 判断启用截止日期是否合法
     * 设置方案名称
     */
    private List<String> getOrgsFromPlan(DynamicObject plan){
        // 过滤条件
        String planNum = plan.getString("billno"); // 获取盘点方案编号
        QFilter[] planFilter = {new QFilter(KEY_BILLNO, QCP.equals, planNum)};

        // 查询字段
        StringBuilder fields = new StringBuilder();
        fields.append(KEY_PLANNAME).append(","); // 方案名称
        fields.append(KEY_PLANENTRY_ENTRY).append(".").append(KEY_ASSETSORG).append(".number").append(","); // 资产组织
        fields.append(KEY_PLANENTRY_ENTRY).append(".").append(KEY_ENDDATE); // 启用截止日期

        // 使用QueryServiceHelper.query查询单据，并且第一个参数(实体标识)设为单据的标识时
        // 若要查询单据体的字段，需要在单据体字段前加“单据体标识.”

        DynamicObjectCollection dynamicObjects = QueryServiceHelper.query(KEY_INVENTORYPLAN_BILL, fields.toString(), planFilter);
        planname = dynamicObjects.get(0).getString(KEY_PLANNAME); // 设置方案名称

        List<String> orgs = new ArrayList<>();
        Date now = new Date();
        // 查询该方案包含的所有组织num
        for (DynamicObject dynamicObject : dynamicObjects) {
            Date endDate = dynamicObject.getDate(KEY_PLANENTRY_ENTRY+"."+KEY_ENDDATE);
            isDateValid = endDate.before(now); // 判断日期合法性

            // 注意此处添加的是org.number，则后面getAssetsFromOrgs中设置过滤时也要用org.number
            orgs.add(dynamicObject.getString(KEY_PLANENTRY_ENTRY+"."+KEY_ASSETSORG+".number"));
        }
        return orgs;
    }
}
