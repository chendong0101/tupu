import { Tag, Space, Menu } from 'antd';
import { QuestionCircleOutlined } from '@ant-design/icons';
import React from 'react';
import { useModel, SelectLang } from 'umi';
import Avatar from './AvatarDropdown';
import HeaderDropdown from '../HeaderDropdown';
import HeaderSearch from '../HeaderSearch';
import styles from './index.less';

export type SiderTheme = 'light' | 'dark';

const ENVTagColor = {
  dev: 'orange',
  test: 'green',
  pre: '#87d068',
};

const GlobalHeaderRight: React.FC<{}> = () => {
  const { initialState } = useModel('@@initialState');

  if (!initialState || !initialState.settings) {
    return null;
  }

  const { navTheme, layout } = initialState.settings;
  let className = styles.right;

  if ((navTheme === 'dark' && layout === 'top') || layout === 'mix') {
    className = `${styles.right}  ${styles.dark}`;
  }
  return (
    <Space className={className}>
      <HeaderSearch
        className={`${styles.action} ${styles.search}`}
        placeholder="站内搜索"
        defaultValue="SQL Search"
        options={[
          {
            label: <a href="sql_server">SQL Search</a>,
            value: 'Tupu SQL Search',
          },
        ]}
        // onSearch={value => {
        //   console.log('input', value);
        // }}
      />
      <HeaderDropdown
        overlay={
          <Menu>
            <Menu.Item
              onClick={() => {
                window.open('http://clbwiki.wenwen.sogou.com/info/86#2184');
              }}
            >
              TUPU 问答文档
            </Menu.Item>
          </Menu>
        }
      >
        <span className={styles.action}>
          <QuestionCircleOutlined />
        </span>
      </HeaderDropdown>
      <SelectLang className={styles.action} />
    </Space>
  );
};
export default GlobalHeaderRight;
