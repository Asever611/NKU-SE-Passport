package qqq6.prj66.asset.assetsmanagement.base.storage;

import kd.bos.base.AbstractBasePlugIn;
import kd.bos.dataentity.entity.DynamicObject;
import kd.bos.dataentity.utils.StringUtils;
import kd.bos.entity.datamodel.events.PropertyChangedArgs;
import kd.bos.orm.ORM;
import kd.bos.orm.query.QFilter;

import java.util.List;

/**
 * 插件
 * 基础资料 - 存放地点
 * 1.拼接地点全称
 */
public class MainPlugin extends AbstractBasePlugIn {

    private static final String KEY_ADMINDIVISION = "qqq6_admindivision"; // 行政区划
    private static final String KEY_ADDRESSDETAIL = "qqq6_addressdetail"; // 详细地址
    private static final String KEY_FULLADDRESS = "qqq6_fulladdress"; // 地址全称

    @Override
    public void propertyChanged(PropertyChangedArgs e) {
        String changedKey = e.getProperty().getName();

        if (StringUtils.equals(changedKey, KEY_ADMINDIVISION) || StringUtils.equals(changedKey, KEY_ADDRESSDETAIL)) {
            concatFullAddress();
        }

        super.propertyChanged(e);
    }

    // ======================== 工具方法 ========================

    /**
     * 拼接地址全称
     */
    private void concatFullAddress() {
        String adminId = (String) this.getModel().getValue(KEY_ADMINDIVISION);
        String addressDetail = (String) this.getModel().getValue(KEY_ADDRESSDETAIL);
        if (StringUtils.isEmpty(adminId) || StringUtils.isEmpty(addressDetail)) {
            return;
        }
        System.out.println(adminId);
        System.out.println(this.getModel().getValue(KEY_ADMINDIVISION));
        String adminName = getAdminDivisionName(adminId);

        this.getModel().setValue(KEY_FULLADDRESS, adminName + addressDetail);
    }

    /**
     * 根据行政区划id查询行政区划全称
     * - 直辖市判定
     * - 删除间隔符
     */
    private String getAdminDivisionName(String adminId) {

        String adminName = "";

        // 条件
        ORM orm = ORM.create();
        QFilter filter = new QFilter("id", "=", Long.parseLong(adminId));
        QFilter[] filters = new QFilter[]{filter};

        // 查询
        List<DynamicObject> dynamicObjects = orm.query("bd_admindivision", "id,name,fullname,country.id,country.name", filters, "id asc");

        if (!dynamicObjects.isEmpty()) {
            DynamicObject dynamicObject = dynamicObjects.get(0);
            adminName = dynamicObject.getString("fullname");
            adminName = adminName.replace("_", "");
            if (adminName.length() >= 4 && StringUtils.equals(adminName.substring(0, 2), adminName.substring(2, 4))) {
                adminName = adminName.substring(2);
            }
            adminName = dynamicObject.getString("country.name") + adminName;
        }

        return adminName;
    }
}
