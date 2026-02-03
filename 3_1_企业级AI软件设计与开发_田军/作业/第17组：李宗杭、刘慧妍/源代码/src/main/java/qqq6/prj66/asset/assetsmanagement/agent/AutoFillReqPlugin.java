package qqq6.prj66.asset.assetsmanagement.agent;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import kd.bos.context.RequestContext;
import kd.bos.dataentity.entity.DynamicObject;
import kd.bos.dataentity.entity.DynamicObjectCollection;
import kd.bos.entity.operate.result.OperationResult;
import kd.bos.form.gpt.IGPTAction;
import kd.bos.orm.query.QCP;
import kd.bos.orm.query.QFilter;
import kd.bos.servicehelper.BusinessDataServiceHelper;
import kd.bos.servicehelper.operation.SaveServiceHelper;
import kd.bos.servicehelper.user.UserServiceHelper;

import java.math.BigDecimal;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

public class AutoFillReqPlugin implements IGPTAction {

    private static final String OPERATION = "GET_REQUEST_JSON";

    private static final String KEY_REQUESTBILL = "qqq6_requestbill";
    private static final String KEY_REQUESTENTRY = "qqq6_requestentry";
    private static final String KEY_REASON = "qqq6_reason";
    private static final String KEY_REQUESTER = "qqq6_requester";
    private static final String KEY_REQDATE = "qqq6_reqdate";
    private static final String KEY_ASSETSNAME = "qqq6_assetsname";
    private static final String KEY_QTY = "qqq6_qty";
    private static final String KEY_UNIT = "qqq6_unit";

    @Override
    public Map<String, String> invokeAction(String s, Map<String, String> map) {

        Map<String, String> result = new HashMap<String, String>();

        if(OPERATION.equals(s)){
            // 将json字符串转换为JSON对象
            String jsonResult = map.get("jsonResult").replaceAll("\\s+", "").trim();
            JSONObject jsonResultObject = null;
            jsonResultObject = JSON.parseObject(jsonResult);

            // 获取出差申请单和单据体数据模型
            DynamicObject requestBill = BusinessDataServiceHelper.newDynamicObject(KEY_REQUESTBILL);
            DynamicObjectCollection requestEntry = requestBill.getDynamicObjectCollection(KEY_REQUESTENTRY);

            // 设置用户
            long currUserId = UserServiceHelper.getCurrentUserId();
            requestBill.set(KEY_REQUESTER,currUserId);

            // 设置组织
            long orgId = RequestContext.get().getOrgId();
            requestBill.set("org", orgId);

            // 设置日期
            Date reqDate = new Date();
            requestBill.set(KEY_REQDATE, reqDate);

            // 设置申请事由
            String reason = jsonResultObject.getString(KEY_REASON);
            requestBill.set(KEY_REASON, reason);

            // 设置单据状态
            requestBill.set("billstatus", "A");

            // 设置单据体
            JSONArray jsonArray = jsonResultObject.getJSONArray(KEY_REQUESTENTRY);
            if (jsonArray !=null && !jsonArray.isEmpty()) {
                for (int i = 0; i < jsonArray.size(); i++) {
                    JSONObject entryJson = jsonArray.getJSONObject(i);
                    // 获取单据体数据类型
                    DynamicObject entry = new DynamicObject(requestEntry.getDynamicObjectType());

                    // 获取数据
                    String assetsName = entryJson.getString(KEY_ASSETSNAME);
                    BigDecimal qty = entryJson.getBigDecimal(KEY_QTY);
                    String unit = entryJson.getString(KEY_UNIT);

                    // 计量单位对象
                    QFilter[] filters = {new QFilter("name", QCP.equals, "个")};
//                    DynamicObject unitDO = BusinessDataServiceHelper.loadSingle("bd_unit", filters);

                    // 设置数据
                    entry.set(KEY_ASSETSNAME, assetsName);
                    entry.set(KEY_QTY, qty);
                    entry.set(KEY_UNIT, 11);

                    // 添加到单据体
                    requestEntry.add(entry);
                }
            }

            // 保存出差申请单
            OperationResult operationResult = SaveServiceHelper.saveOperate(KEY_REQUESTBILL, new DynamicObject[]{requestBill}, null);

            // 保存结果消息
            String message = operationResult.getMessage();

            // 获取当前保存数据对应的ID
            Long pkId = (Long)requestBill.getPkValue();

            // 拼接超链接
            String targetForm = "bizAction://currentPage?gaiShow=1&selectedProcessNumber=processNumber&gaiAction=showBillForm&gaiParams={\"appId\":\"qqq6_assetsmanagement\",\"billFormId\":\"qqq6_requestbill\",\"billPkId\":\"" + pkId + "\"}&title=资产领用申请单&iconType=bill&method=bizAction";

            // 返回领用申请单链接
            result.put("formURL", targetForm);
            result.put("message", message);
            result.put("str", "执行成功");
        }
        return result;
    }

}
