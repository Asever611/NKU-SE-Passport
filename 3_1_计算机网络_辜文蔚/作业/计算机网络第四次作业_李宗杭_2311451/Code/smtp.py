import smtplib
from email.mime.text import MIMEText
from email.header import Header

# 第三方SMTP服务
mail_host = "smtp.qq.com"  # 设置服务器
# 自己的邮箱，通过QQ邮箱设置获取口令
mail_user = "2723733688@qq.com"  # 发送方：QQ邮箱
mail_pass = ""  # 获取授权码
sender = "2723733688@qq.com"  # 发送方qq邮箱
receivers = ["13332033020@163.com"]  # 接受者的邮箱,另一个邮箱

# class email.mime.text.MIMEText(_text[, _subtype[, _charset]]) 用于创建主要类型文本的MIME对象，_text是有效负载的字符串,_subtype
# 是次要类型，默认为plain._charset是文本的字符集
message = MIMEText('I love computer networks', 'plain', 'utf-8')
message["From"] = Header(f"lizonghang <{sender}>")  # 显示名和发件人邮箱
message["To"] = Header(f"<{receivers[0]}>", "utf-8")  # 收件人邮箱
message['Subject'] = Header("邮件测试")  # 邮件主题

try:
    smtpObj = smtplib.SMTP_SSL(mail_host, 465)
    smtpObj.login(mail_user, mail_pass)  # 会返回(状态码, "字符串解释")元组信息
    smtpObj.sendmail(sender, receivers, message.as_string())
    print("邮件发送成功")
except smtplib.SMTPException as e:
    print(e)
    print("Error: 无法发送邮件")

