import React from 'react';
import { GithubOutlined } from '@ant-design/icons';
import { DefaultFooter } from '@ant-design/pro-layout';

export default () => (
  <DefaultFooter
    copyright="2020 sogou 社区搜索团队"
    links={[
      {
        key: 'github',
        title: <GithubOutlined />,
        href: 'https://git.sogou-inc.com/wenwen_backend/tupu_arch',
        blankTarget: true,
      },
    ]}
  />
);
