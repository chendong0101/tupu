import React from 'react';
import { PageContainer } from '@ant-design/pro-layout';
import { Anchor, Card, Alert, Typography } from 'antd';
import { useIntl, FormattedMessage } from 'umi';
import styles from './Welcome.less';

const CodePreview: React.FC<{}> = ({ children }) => (
  <pre className={styles.pre}>
    <code>
      <Typography.Text copyable>{children}</Typography.Text>
    </code>
  </pre>
);

export default (): React.ReactNode => {
  const intl = useIntl();
  return (
    <PageContainer>
      <Card>
        <Alert
          message='更快更强的 TupuSearch New Arch 已经发布。'
          type="success"
          showIcon
          banner
          style={{
            margin: -12,
            marginBottom: 24,
          }}
        />
        <Typography.Text strong>
           Wiki
        </Typography.Text>
        <Anchor>
            <Anchor.Link href = "http://clbwiki.wenwen.sogou.com/info/86#2184" title = "http://clbwiki.wenwen.sogou.com/info/86#2184"/>
            <Anchor.Link href = "http://clbwiki.wenwen.sogou.com/info/89#2548" title = "http://clbwiki.wenwen.sogou.com/info/89#2548"/>
        </Anchor>
      </Card>
    </PageContainer>
  );
};
